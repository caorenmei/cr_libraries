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
            // 解析包
            std::string typeName = value.c_str();
            const char* data = value.data() + typeName.size() + 1;
            std::size_t dataLen = value.size() - (typeName.size() + 1);
            // 序列化
            if (typeName == pb::OpCommand::descriptor()->full_name())
            {
                pb::OpCommand command;
                if (command.ParseFromArray(data, dataLen))
                {
                    onOpCommand(command);
                }
                else
                {
                    CRLOG_WARN(logger_, "NameService") << "Parse Command Failed: " << typeName;
                }
            }
            else
            {
                CRLOG_WARN(logger_, "NameService") << "Unknow Command: " << typeName;
            }
        }

        void NameService::onOpCommand(const pb::OpCommand& command)
        {
            // 路径
            if (command.path_size() == 0)
            {
                CRLOG_WARN(logger_, "NameService") << "Command Path Size: " << command.path_size();
                return;
            }
            std::vector<std::string> path;
            std::copy(command.path().begin(), command.path().end(), std::back_inserter(path));
            // 客户端Id
            boost::uuids::uuid clientId;
            if (clientId.size() != command.client_id().size())
            {
                CRLOG_WARN(logger_, "NameService") << "Command Client Id Format Failed: " << command.client_id().size();
                return;
            }
            std::memcpy(clientId.data, command.client_id().data(), clientId.size());
            // 操作Id
            boost::uuids::uuid commandId;
            if (commandId.size() != command.client_id().size())
            {
                CRLOG_WARN(logger_, "NameService") << "Command Command Id Format Failed: " << command.command_id().size();
                return;
            }
            std::memcpy(commandId.data, command.command_id().data(), commandId.size());
            // 消息类型
            switch (command.op())
            {
            case pb::ADD:
                onAddCommand(clientId, commandId, path, command);
                break;
            case pb::UPDATE:
                onUpdateCommand(clientId, commandId, path, command);
                break;
            case pb::REMOVE:
                onRemoveCommand(clientId, commandId, path, command);
                break;
            default: break;
            }
        }

        void NameService::onAddCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
            const std::vector<std::string>& path, const pb::OpCommand& command)
        {
            auto parent = getParentValueNode(path);
            // 父节点不存在
            if (parent == nullptr)
            {
                CRLOG_WARN(logger_, "NameService") << "Add Parent Node Not Exists, Path = " << pathToString(path);
                return;
            }
            // 子节点已存在
            if (parent->children.count(path.back()) != 0)
            {
                CRLOG_WARN(logger_, "NameService") << "Add Node Already Exists, Path = " << pathToString(path);
                return;
            }
            // 父节点是临时节点
            if ((parent->mode & pb::EPHEMERAL) != 0)
            {
                CRLOG_WARN(logger_, "NameService") << "Add Parent Node Is Ephemeral Node, Version = " << parent->cversion << "!=" << command.version() << ", Path = " << pathToString(path);
                return;
            }
            // 版本号不匹配
            if ((command.mode() & pb::SEQUENTIAL) != 0 && parent->version != command.version()
                || parent->cversion != command.cversion())
            {
                CRLOG_WARN(logger_, "NameService") << "Add Parent Node Version Mismatch, Version = " << parent->cversion << "!=" << command.version() << ", Path = " << pathToString(path);
                return;
            }
            // 递增版本号
            versionIndex_ = versionIndex_ + 1;
            // 节点名字
            std::string name = path.back();
            if ((command.mode() & pb::SEQUENTIAL) != 0)
            {
                std::stringstream sstrm;
                sstrm << std::setfill('0') << std::setw(10) << parent->sequential;
                parent->sequential = parent->sequential + 1;
                name += sstrm.str();
            }
            // 构造节点
            auto node = std::make_shared<ValueNode>();
            node->name = name;
            node->parent = parent;
            node->version = versionIndex_;
            node->cversion = versionIndex_;
            node->owner = clientId;
            node->mode = command.mode();
            node->value = command.value();
            // 加入到父节点
            parent->children.insert(std::make_pair(name, node));
            parent->cversion = versionIndex_;
            // 临时节点，加入到列表
            if ((command.mode() & pb::EPHEMERAL) != 0)
            {
                auto& ephemerals = ephemerals_[clientId];
                ephemerals.first.insert(node);
                ephemerals.second = 0;
            }
            // 完成
            CRLOG_DEBUG(logger_, "NameService") << "Add Node Success: " << pathToString(path);
        }

        void NameService::onUpdateCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
            const std::vector<std::string>& path, const pb::OpCommand& command)
        {
            auto node = getValueNode(path);
            // 节点不存在
            if (node == nullptr)
            {
                CRLOG_WARN(logger_, "NameService") << "Update Node Not Exists, Path = " << pathToString(path);
                return;
            }
            // 版本号不匹配
            if (node->version != command.version())
            {
                CRLOG_WARN(logger_, "NameService") << "Update Node Version Mismatch, Version = " << node->version << "!=" << command.version() << ", Path = " << pathToString(path);
                return;
            }
            // 递增版本号
            versionIndex_ = versionIndex_ + 1;
            // 更新
            node->value = command.value();
            node->version = versionIndex_;
            // 完成
            CRLOG_DEBUG(logger_, "NameService") << "Update Node Success: " << pathToString(path);
        }

        void NameService::onRemoveCommand(const boost::uuids::uuid& clientId, const boost::uuids::uuid& commandId,
            const std::vector<std::string>& path, const pb::OpCommand& command)
        {
            auto node = getValueNode(path);
            auto parent = node->parent.lock();
            // 节点不存在
            if (node == nullptr)
            {
                CRLOG_WARN(logger_, "NameService") << "Remove Node Not Exists, Path = " << pathToString(path);
                return;
            }
            // 版本号不匹配
            if (node->version != command.version())
            {
                CRLOG_WARN(logger_, "NameService") << "Remve Node Version Mismatch, Version = " << node->version << "!=" << command.version() << ", Path = " << pathToString(path);
                return;
            }
            // 递增版本号
            versionIndex_ = versionIndex_ + 1;
            // 删除
            parent->children.erase(node->name);
            parent->cversion = versionIndex_;
            // 如果是临时节点，从临时队列移除
            if ((node->mode & pb::EPHEMERAL) != 0)
            {
                auto ephemeralIter = ephemerals_.find(node->owner);
                assert(ephemeralIter != ephemerals_.end());
                auto nodeIter = ephemeralIter->second.first.find(node);
                assert(nodeIter != ephemeralIter->second.first.end());
                ephemeralIter->second.first.erase(nodeIter);
                if (ephemeralIter->second.first.empty())
                {
                    ephemerals_.erase(ephemeralIter);
                }
            }
            // 完成
            CRLOG_DEBUG(logger_, "NameService") << "Remove Node Success: " << pathToString(path);
        }

        std::shared_ptr<NameService::ValueNode> NameService::getValueNode(const std::vector<std::string>& path) const
        {
            std::shared_ptr<ValueNode> node = root_;
            for (std::size_t depth = 0; depth < path.size() - 1 && node != nullptr; ++depth)
            {
                auto iter = node->children.find(path[depth]);
                if (iter != node->children.end())
                {
                    node = iter->second;
                }
                else
                {
                    node = nullptr;
                }
            }
            return node;
        }

        std::shared_ptr<NameService::ValueNode> NameService::getParentValueNode(const std::vector<std::string>& path) const
        {
            std::shared_ptr<ValueNode> parent = root_;
            for (std::size_t depth = 0; depth < path.size() - 1 && parent != nullptr; ++depth)
            {
                auto iter = parent->children.find(path[depth]);
                if (iter != parent->children.end())
                {
                    parent = iter->second;
                }
                else
                {
                    parent = nullptr;
                }
            }
            return parent;
        }

    }
}