#include "name_service.h"

#include <cassert>
#include <ctime>
#include <limits>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace cr
{
    namespace app
    {
        // 节点定义
        struct NameService::ValueNode
        {
            // 创建类型
            enum CreateMode
            {
                // 持久节点
                PERSISTENT = 1,
                // 持久持久顺序节点
                PERSISTENT_SEQUENTIAL = 2,
                // 临时节点
                EPHEMERAL = 3,
                // 临时顺序节点
                EPHEMERAL_SEQUENTIAL = 4,
            };
            // 父节点
            std::weak_ptr<ValueNode> parent;
            // 版本
            std::uint64_t version;
            // 所属客户端
            boost::uuids::uuid owner;
            // 节点类型
            CreateMode mode;
            // 序号
            std::uint64_t sequential;
            // 节点名
            std::string name;
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

        NameService::NameService(cr::app::Application& context, boost::asio::io_service& ioService,
            std::uint32_t id, std::string name, const Options& options)
            : RaftService(context, ioService, id, name, options),
            ioService_(ioService),
            logger_(RaftService::getLogger()),
            options_(options),
            tickTimer_(ioService_),
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
            std::string typeName = value.c_str();
            const char* data = value.data() + typeName.size() + 1;
            std::size_t dataLen = value.size() - (typeName.size() + 1);
        }
    }
}