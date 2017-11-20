#ifndef CR_COMMON_RAFT_RAFT_NODE_H_
#define CR_COMMON_RAFT_RAFT_NODE_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <memory>

#include <boost/asio.hpp>

#include <cr/core/logging.h>
#include <cr/network/connector.h>
#include <cr/network/pb_connection.h>

#include "raft.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {

        /** 前置声明消息 */
        namespace pb
        {
            /** 握手请求 */
            class RaftHandshakeReq;
            /** 握手回复 */
            class RaftHandshakeResp;
            /** 提交请求 */
            class RaftProposeReq;
            /** 提交回复 */
            class RaftProposeResp;
        }

        /** 一个raft节点 */
        class RaftNode : public std::enable_shared_from_this<RaftNode>
        {
        public:

            /** 连接改变回调 */
            using ConnectCallback = std::function<void(const std::shared_ptr<RaftNode>&)>;

            /** 更新状态机回调 */
            using UpdateCallback = std::function<void(const std::shared_ptr<RaftNode>&)>;

            /**
             * 构造函数
             * @param ioService io service
             * @param logger 日志
             * @param raft raft
             * @param id 节点Id
             * @param endpoint 远程地址
             */
            RaftNode(boost::asio::io_service& ioService, cr::log::Logger& logger, Raft& raft, std::uint64_t id, 
                const boost::asio::ip::tcp::endpoint& endpoint);

            /** 析构函数 */
            ~RaftNode();

            RaftNode(const RaftNode&) = delete;
            RaftNode& operator=(const RaftNode&) = delete;

            /**
             * 设置连接改变回调
             * @param cb 回调
             */
            void setConnectCallback(ConnectCallback cb);

            /**
             * 设置更新状态机回调
             * @param cb 回调
             */
            void setUpdateCallback(UpdateCallback cb);

            /**
             * 获取节点Id
             * @return 节点Id
             */ 
            std::uint64_t getId() const;

            /**
             * 节点是不是领导节点
             * @return true是，false其他
             */
            bool isLeader() const;

            /**
             * 节点是否连接
             * @return true是，false其他
             */
            bool isConnected() const;

            /** 启动 */
            void start();

            /** 停止 */
            void stop();

            /**
             * 有客户端连接上
             * @param conn 连接
             */
            void onConnect(const std::shared_ptr<cr::network::PbConnection>& conn);

            /**
             * 连接断开
             * @param conn 连接
             */
            void onDisconnect(const std::shared_ptr<cr::network::PbConnection>& conn);

            /**
             * 接受到消息
             * @param message 消息
             */
            void onMessage(const std::shared_ptr<google::protobuf::Message>& message);

            /**
             * 发送消息
             * @param message 消息
             */
            void sendMessage(const std::shared_ptr<google::protobuf::Message>& message);

            /**
             * 执行一条指令
             * @param value 指令值
             * @param cb 回调
             */
            bool execute(const std::string& value, std::function<void(std::uint64_t, int)> cb);

        private:

            // 连接建立
            void onConnectHandler(boost::asio::ip::tcp::socket socket);
            
            // 连接建立
            void onConnectHandler(const std::shared_ptr<cr::network::PbConnection>& conn);

            // 连接断开
            void onDisconnectHandler(const std::shared_ptr<cr::network::PbConnection>& conn);

            // 连接断开
            void onDisconnect();

            // 消息处理
            void onMessageHandler(const std::shared_ptr<google::protobuf::Message>& message);

            // 握手回执
            void onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftHandshakeResp>& message);

            // 转发消息
            void onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftProposeReq>& message);

            // 转发回执
            void onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftProposeResp>& message);

            // raft 消息
            void onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // io service
            boost::asio::io_service& ioService_;
            // 日志
            cr::log::Logger& logger_;
            // raft 状态机
            Raft& raft_;
            // 节点Id
            std::uint64_t id_;
            // 节点地址
            boost::asio::ip::tcp::endpoint endpoint_;
            // 运行状态
            bool running_;
            // 连接改变回调
            ConnectCallback connectCallback_;
            // 更新回调
            UpdateCallback updateCallback_;
            // 连接器
            std::shared_ptr<cr::network::Connector> connector_;
            // 连接
            std::shared_ptr<cr::network::PbConnection> connection_;
            // 是否连接
            bool connected_;
            // 消息序号
            std::uint64_t serialNo_;
            // 回调
            std::map<std::uint64_t, std::function<void(std::uint64_t, int)>> callbacks_;
        };
    }
}

#endif
