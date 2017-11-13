#include "name_service.h"

#include <cassert>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace cr
{
    namespace app
    {
        // 节点定义
        struct NameService::ValueNode
        {
            // 节点名
            std::string name;
            // 父节点
            std::weak_ptr<ValueNode> parent;
            // 数据版本
            std::uint64_t version = 0;
            // 数据版本
            std::uint64_t dversion = 0;
            // 子节点版本
            std::uint64_t cversion = 0;
            // 所属客户端
            boost::uuids::uuid owner;
            // 节点类型
            uint32_t mode = pb::PERSISTENT;
            // 序号
            std::uint64_t sequential = 0;
            // 值
            std::string value;
            // 子节点
            std::map<std::string, std::shared_ptr<ValueNode>> children;

			// 获取子节点
			static std::shared_ptr<ValueNode> getChild(const std::shared_ptr<ValueNode>& root, const std::vector<std::string>& path)
			{
				std::shared_ptr<ValueNode> node = root;
				for (std::size_t i = 0; i < path.size() && node != nullptr; ++i)
				{
					auto iter = node->children.find(path[i]);
					node = iter != node->children.end() ? iter->second : nullptr;
				}
				return node;
			}

			// 构造节点
			static std::shared_ptr<ValueNode> makeValueNode(std::string name, std::uint64_t version, const pb::OpCommand& command)
			{
				auto node = std::make_shared<ValueNode>();
				node->name = name;
				node->version = version;
				node->cversion = version;
				node->mode = command.mode();
				node->value = command.value();
				return node;
			}
        };

        // 连接定义
        struct NameService::Connection
        {
            // 连接
            std::shared_ptr<cr::network::PbConnection> conn;
            // 消息
            std::map<std::uint64_t, std::shared_ptr<google::protobuf::Message>> messages;
            // 消息 - >uuid
            std::map<std::uint64_t, std::set<boost::uuids::uuid>> msgUuids;
            // uuid->消息
            std::map<boost::uuids::uuid, std::uint64_t> uuidMsgs;
        };

        // 路径到字符串
        static std::string pathToString(const std::vector<std::string>& strings)
        {
            std::string text = "[ ";
            for (std::size_t i = 0; i != strings.size(); ++i)
            {
                text.append(strings[i]);
                text.append(", ");
            }
            text.append("]");
            return text;
        }

        NameService::NameService(cr::app::Application& context, boost::asio::io_service& ioService,
            std::uint32_t id, std::string name, const Options& options)
            : RaftService(context, ioService, id, name, options),
            ioService_(ioService),
            logger_(RaftService::getLogger()),
            options_(options),
            tickTimer_(ioService_),
            versionIndex_(0),
            root_(std::make_shared<ValueNode>()),
            acceptor_(ioService_),
            socket_(ioService_),
            nextId_(0)
        {}
            
        NameService::~NameService()
        {}

        void NameService::onStart()
        {
            RaftService::onStart();
        }

        void NameService::onStop()
        {
            RaftService::onStop();
        }

        void NameService::onCommand(std::uint64_t index, const std::string& value)
        {
            CRLOG_DEBUG(logger_, "NameService") << "Handle Command: " << index << "->[length:" << value.size() << "]";
            // 序列化
            pb::OpCommand command;
            if (!command.ParseFromString(value))
            {
                CRLOG_WARN(logger_, "NameService") << "Parse Command Failed";
                return; 
            }
            onOpCommand(command);
        }

        void NameService::onOpCommand(const pb::OpCommand& command)
        {
            // 客户端Id
            boost::uuids::uuid clientId;
            std::memcpy(clientId.data, command.client_id().data(), std::min(clientId.size(), command.client_id().size()));
            // 操作Id
            boost::uuids::uuid commandId;
            std::memcpy(commandId.data, command.command_id().data(), std::min(commandId.size(), command.command_id().size()));
            // 处理消息
			std::uint32_t result = pb::ERR_SUCCESS;
			std::shared_ptr<ValueNode> node;
            switch (command.op())
            {
            case pb::ADD:
				result = onAddCommand(clientId, commandId, command, node);
                break;
            case pb::UPDATE:
				result = onUpdateCommand(clientId, commandId, command, node);
                break;
            case pb::REMOVE:
				result = onRemoveCommand(clientId, commandId, command, node);
                break;
            default: 
				assert(!"invalid code path");
				break;
            }
			// 客户端连接
			auto connIter = connections_.find(clientId);
			if (connIter == connections_.end())
			{
				return;
			}
			auto& conn = connIter->second;
			// 查找指令消息
			auto commandIter = conn->uuidMsgs.find(commandId);
			if (commandIter == conn->uuidMsgs.end())
			{
				return;
			}
			auto msgIndex = commandIter->second;
			// 消息
			auto msgIter = conn->messages.find(msgIndex);
			assert(msgIter != conn->messages.end());
			auto message = msgIter->second;
			// 删除消息
			conn->messages.erase(msgIter);
			conn->msgUuids.erase(msgIndex);
			conn->uuidMsgs.erase(commandIter);
			// 处理消息
        }

        std::uint32_t NameService::onAddCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId, 
			const pb::OpCommand& command, std::shared_ptr<ValueNode>& node)
        {
			// 获取父节点
			std::vector<std::string> path(command.path().begin(), command.path().end());
			auto parent = ValueNode::getChild(root_, path);
			// 父节点不存在
			if (parent == nullptr)
			{
				CRLOG_WARN(logger_, "NameService") << "Add Parent Node Not Exists, Path = " << pathToString(path);
				return pb::ERR_PARENT_NOT_EXISTS;
			}
			// 父节点是临时节点
			if ((parent->mode & pb::EPHEMERAL) != 0)
			{
				CRLOG_WARN(logger_, "NameService") << "Add Parent Node Is Ephemeral Node, Path = " << pathToString(path);
				return pb::ERR_PARENT_IS_EPHEMERAL;
			}
			// 版本号不匹配
			if ((command.mode() & pb::SEQUENTIAL) != 0 && parent->version != command.version())
			{
				CRLOG_WARN(logger_, "NameService") << "Add Parent Node Version Mismatch, Path = " << pathToString(path);
				return pb::ERR_VERSION_MISMATCH;
			}
			else if (parent->cversion != command.cversion())
			{
				CRLOG_WARN(logger_, "NameService") << "Add Parent Child Version Mismatch, Path = " << pathToString(path);
				return pb::ERR_CHILD_VERSION_MISMATCH;
			}
			// 子节点已存在
			if (parent->children.count(command.name()) != 0)
			{
				CRLOG_WARN(logger_, "NameService") << "Add Node Already Exists, Path = " << pathToString(path);
				return pb::ERR_NODE_EXISTS;
			}
			// 递增版本号
			versionIndex_ = versionIndex_ + 1;
			// 节点名字
			std::string name = command.name();
			if ((command.mode() & pb::SEQUENTIAL) != 0)
			{
				std::stringstream sstrm;
				sstrm << std::setfill('0') << std::setw(10) << parent->sequential;
				parent->sequential = parent->sequential + 1;
				name += sstrm.str();
			}
			// 构造节点
			node = ValueNode::makeValueNode(name, versionIndex_, command);
			node->parent = parent;
			// 加入到父节点
			parent->children.insert(std::make_pair(name, node));
			parent->cversion = versionIndex_;
			// 临时节点，加入到列表
			if ((command.mode() & pb::EPHEMERAL) != 0)
			{
				node->owner = clientId;
				addAutoDeleteNode(node);
			}
			// 完成
			CRLOG_DEBUG(logger_, "NameService") << "Add Node Success: " << pathToString(path);
			return pb::ERR_SUCCESS;
        }

		std::uint32_t NameService::onUpdateCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
			const pb::OpCommand& command, std::shared_ptr<ValueNode>& node)
        {
			// 获取节点
			std::vector<std::string> path(command.path().begin(), command.path().end());
			path.push_back(command.name());
			node = ValueNode::getChild(root_, path);
			// 节点不存在
			if (node == nullptr)
			{
				CRLOG_WARN(logger_, "NameService") << "Update Node Not Exists, Path = " << pathToString(path);
				return pb::ERR_NODE_NOT_EXISTS;
			}
			// 版本号不匹配
			if (node->dversion != command.dversion())
			{
				CRLOG_WARN(logger_, "NameService") << "Update Node Version Mismatch, Path = " << pathToString(path);
				return pb::ERR_DATA_VERSION_MISMATCH;
			}
			// 递增版本号
			versionIndex_ = versionIndex_ + 1;
			// 更新
			node->value = command.value();
			node->version = versionIndex_;
			// 完成
			CRLOG_DEBUG(logger_, "NameService") << "Update Node Success: " << pathToString(path);
			return pb::ERR_SUCCESS;
        }

		std::uint32_t NameService::onRemoveCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
			const pb::OpCommand& command, std::shared_ptr<ValueNode>& node)
        {
			// 获取节点
			std::vector<std::string> path(command.path().begin(), command.path().end());
			path.push_back(command.name());
			node = ValueNode::getChild(root_, path);
			// 节点不存在
			if (node == nullptr)
			{
				CRLOG_WARN(logger_, "NameService") << "Remove Node Not Exists, Path = " << pathToString(path);
				return pb::ERR_NODE_NOT_EXISTS;
			}
			// 版本号不匹配
			if (node->version != command.version())
			{
				CRLOG_WARN(logger_, "NameService") << "Remve Node Version Mismatch, Path = " << pathToString(path);
				return pb::ERR_DATA_VERSION_MISMATCH;
			}
			// 递增版本号
			versionIndex_ = versionIndex_ + 1;
			// 删除
			auto parent = node->parent.lock();
			parent->children.erase(node->name);
			parent->cversion = versionIndex_;
			// 如果是临时节点，从临时队列移除
			if ((node->mode & pb::EPHEMERAL) != 0)
			{
				removeAutoDeleteNode(node);
			}
			// 完成
			CRLOG_DEBUG(logger_, "NameService") << "Remove Node Success: " << pathToString(path);
			return pb::ERR_SUCCESS;
        }

		void NameService::addAutoDeleteNode(const std::shared_ptr<ValueNode>& node)
		{
			auto& ephemerals = ephemerals_[node->owner];
			ephemerals.first.insert(node);
			ephemerals.second = 0;
		}

		void NameService::removeAutoDeleteNode(const std::shared_ptr<ValueNode>& node)
		{
			// 客户端列表
			auto ephemeralIter = ephemerals_.find(node->owner);
			assert(ephemeralIter != ephemerals_.end());
			// 节点列表
			auto nodeIter = ephemeralIter->second.first.find(node);
			assert(nodeIter != ephemeralIter->second.first.end());
			// 删除节点
			ephemeralIter->second.first.erase(nodeIter);
			if (ephemeralIter->second.first.empty())
			{
				ephemerals_.erase(ephemeralIter);
			}
		}
    }
}