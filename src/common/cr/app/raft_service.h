#ifndef CR_COMMON_APP_RAFT_SERVICE_H_
#define CR_COMMON_APP_RAFT_SERVICE_H_

#include <random>
#include <map>
#include <set>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <rocksdb/db.h>

#include <cr/core/logging.h>
#include <cr/network/connector.h>
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
             * @param cb 日志执行回调
             * @return true成功，false失败
             */
            bool execute(const std::string& value, std::function<void(std::uint64_t, int)> cb);

        private:

            // 监听套接字
            void accept();

            // 客户端套接字
            void onAcceptHandler();

            // 远程服务器断开
            void onPeerDisconectedHandler(const std::shared_ptr<cr::network::PbConnection>& conn);

            // 连接其他节点
            void connect();

            // 连接其他节点
            void connect(std::size_t index);

            // 连接成功回调
            void onClientConnectedHandler(std::size_t index, const std::shared_ptr<cr::network::PbConnection>& conn);

            // 客户端断开连接
            void onClientDisconnectHandler(std::size_t index, const std::shared_ptr<cr::network::PbConnection>& conn);

            // 其他服务消息回调
            void onPeerMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn, 
                const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 握手消息
            void onPeerHandshakeReqHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
                const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 客户端消息回调
            void onClientMessageHandler(std::size_t index, const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 握手回复消息
            void onClientHandshakeRespHandler(std::size_t index, const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 提交请求消息
            void onClientProposeReqHandler(std::size_t index, const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 提交请求回复消息
            void onClientProposeRespHandler(std::size_t index, const std::shared_ptr<cr::raft::pb::RaftMsg>& message);

            // 获取伙伴节点Id
            std::size_t getBuddyNodeIndex(std::uint64_t nodeId) const;

            // Raft 算法逻辑
            void update();

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
            // 数据版本
            std::uint64_t version_;
            // 心跳定时器
            boost::asio::steady_timer timer_;
            // tcp监听器
            boost::asio::ip::tcp::acceptor acceptor_;
            boost::asio::ip::tcp::socket socket_;
            // 服务连接
            std::set<std::shared_ptr<cr::network::PbConnection>> peers_;
            // 连接器
            std::vector<std::shared_ptr<cr::network::Connector>> connectors_;
            // 其他节点连接
            std::vector<std::shared_ptr<cr::network::PbConnection>> connections_;
            // 回调
            std::vector<std::map<std::uint32_t, std::function<void(std::uint64_t, int)>>> callbacks_;
            // 消息队列
            std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages_;
        };
    }
}

#endif
