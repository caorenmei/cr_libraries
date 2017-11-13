#ifndef CR_COMMON_APP_RAFT_SERVICE_H_
#define CR_COMMON_APP_RAFT_SERVICE_H_

#include <random>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <rocksdb/db.h>

#include <cr/core/logging.h>
#include <cr/network/pb_connection.h>
#include <cr/raft/raft.h>

#include "service.h"

namespace cr
{
    namespace app
    {
        /** Raft服务基类 */
        class RaftService : public Service
        {
        public:

            /** Raft服务参数 */
            struct Options
            {
                /** 服务列表,格式: tcp://host:port/id -> tcp://127.0.0.1:3348/1 */
                std::vector<std::string> servers;
                /** 自己Id */
                std::uint64_t myId;
                /** 最小选举超时时间 */
                std::uint64_t minElectionTime;
                /** 最大选举超时时间 */
                std::uint64_t maxElectionTime;
                /** 心跳时间 */
                std::uint64_t heatbeatTime;
                // 日志路径
                std::string binLogPath;
            };

            /**
             * 构造函数
             * @param context app
             * @param ioService 工作线程
             * @param id 服务Id
             * @param name 服务名字
             * @param options 服务参数
             */
            RaftService(cr::app::Application& context, boost::asio::io_service& ioService,
                std::uint32_t id, std::string name, const Options& options);

            /** 析构函数 */
            ~RaftService();

        protected:

            /**
             * 获取日志
             * @return 日志
             */
            cr::log::Logger& getLogger();

            /**
             * 获取服务参数
             * @return 服务参数
             */
            const Options& getOptions() const;

            /** 开始服务 */
            virtual void onStart() override;

            /** 停止服务 */
            virtual void onStop() override;

            /**
             * 运行raft指令
             * @param index 日志index
             * @param value 日志值
             */
            virtual void onCommand(std::uint64_t index, const std::string& value) = 0;

            /** 领导者连接上 */
            virtual void onLeaderConnected();

            /** 领导者断开 */
            virtual void onLeaderDisconnected();

            /**
             * 判断领导者是否连接
             * @return true领导者连接，false其他
             */
            bool isLeaderConnected();

			/**
			 * 获取raft状态
			 */
			const cr::raft::IRaftState& getState() const;

            /**
             * 提交日志
             * @param value 日志数据
             * @return true成功，false失败
             */
            bool propose(const std::vector<std::string>& values);

        private:

            // 第一次运行raft
            void firstRunRaft();

            // 执行raft逻辑
            void runRaft();

            // raft tick 定时器
            void onRaftTimerHandler();

            // 运行raft定时器
            void runRaftTimer(std::uint64_t expiresTime);

            // 停止raft
            void shutdownRaft();

            // 投递第一次监听
            void firstRunAccept();

            // 客户端套接字
            void onAcceptHandler();

            // 监听套接字
            void runAccept();

            // 远程服务器断开
            void onPeerDisconectHandler(const std::shared_ptr<cr::network::PbConnection>& conn);

            // 停止其它节点连接
            void shutdownPeers();

            // 第一次连接服务器
            void firstRunConnect();

            // 连接客户端回调
            void onConnectHandler(std::size_t index);

            // 连接其他服务器节点
            void runConnect(std::size_t index);

            // 连接定时器
            void onConnectTimerHandler();

            // 运行连接定时器
            void runConnectTimer();

            // 客户端断开连接
            void onClientDisconnectHandler(std::size_t index, const std::shared_ptr<cr::network::PbConnection>& conn);

            // 停止客户端连接
            void shutdownClients();

            // 消息处理器
            void onMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message);

            // 消息处理器
            void onRaftMsgHandler(const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 处理提交消息
            void onProposeHandler(const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 发送消息
            bool sendRaftMsg(const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // io service
            boost::asio::io_service& ioService_;
            // 日志
            cr::log::Logger logger_;
            // 服务参数
            Options options_;
            // 随机数
            std::mt19937 random_;
            // 本地文件
            rocksdb::DB* rocksdb_;
            // 状态机
            std::unique_ptr<cr::raft::Raft> raft_;
            // 心跳定时器
            boost::asio::steady_timer tickTimer_;
            // tcp监听器
            boost::asio::ip::tcp::acceptor acceptor_;
            boost::asio::ip::tcp::socket socket_;
            // 服务连接
            std::vector<std::shared_ptr<cr::network::PbConnection>> peers_;
            // 客户端连接
            boost::asio::steady_timer connectTimer_;
            std::vector<boost::asio::ip::tcp::endpoint> servers_;
            std::vector<std::unique_ptr<boost::asio::ip::tcp::socket>> connectors_;
            std::vector<std::shared_ptr<cr::network::PbConnection>> clients_;
            // 消息队列
            std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages_;
        };
    }
}

#endif
