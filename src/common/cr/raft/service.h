﻿#ifndef CR_COMMON_RAFT_SERVICE_H_
#define CR_COMMON_RAFT_SERVICE_H_

#include <random>
#include <map>
#include <set>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <rocksdb/db.h>

#include <cr/app/service.h>
#include <cr/core/logging.h>
#include <cr/network/acceptor.h>
#include <cr/network/connection.h>
#include <cr/network/protobuf_codec.h>

#include "raft.h"
#include "raft_node.h"
#include "service_options.h"

namespace cr
{
    namespace raft
    {
        /** 前置声明消息 */
        namespace pb
        {
            /** 握手请求 */
            class RaftHandshakeReq;
        }

        /** Raft服务基类 */
        class Service : public cr::app::Service
        {
        public:

            /** Raft服务参数 */
            using Options = ServiceOptions;

            /**
             * 构造函数
             * @param context app
             * @param ioService 工作线程
             * @param id 服务Id
             * @param name 服务名字
             * @param options 服务参数
             */
            Service(cr::app::Application& context, boost::asio::io_service& ioService,
                std::uint32_t id, std::string name, const Options& options);

            /** 析构函数 */
            ~Service();

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

            // 客户端套接字
            void onConnectHandler(boost::asio::ip::tcp::socket socket);

            // 客户端套接字
            void onConnectHandler(const std::shared_ptr<cr::network::Connection>& conn);

            // 远程服务器断开
            void onDisconectHandler(const std::shared_ptr<cr::network::Connection>& conn);

            // 消息处理器
            void onMessageHandler(const std::shared_ptr<cr::network::Connection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message);

            // 消息解析失败
            void onMessageErrorHandler(const std::shared_ptr<cr::network::Connection>& conn);

            // 握手消息
            void onMessageHandler(const std::shared_ptr<cr::network::Connection>& conn, const std::shared_ptr<pb::RaftHandshakeReq>& message);

            // 节点连接回调
            void onNodeConnectHandler(const std::shared_ptr<RaftNode>& node);

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
            std::unique_ptr<rocksdb::DB> rocksdb_;
            // 状态机
            std::unique_ptr<Raft> raft_;
            // 心跳定时器
            boost::asio::steady_timer timer_;
            // tcp服务器
            std::shared_ptr<cr::network::Acceptor> acceptor_;
            // 服务连接
            std::map<std::shared_ptr<cr::network::Connection>, std::shared_ptr<RaftNode>> connections_;
            // 消息编解码器
            cr::network::ProtobufCodec codec_;
            // 节点
            std::map<std::uint64_t, std::shared_ptr<RaftNode>> nodes_;
            // 消息队列
            std::vector<std::shared_ptr<cr::raft::pb::RaftMsg>> messages_;
        };
    }
}

#endif
