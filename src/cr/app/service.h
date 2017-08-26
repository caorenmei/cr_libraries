#ifndef CR_SERVICE_H_
#define CR_SERVICE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

#include <boost/asio/io_service.hpp>

#include <cr/concurrent/pipe.h>

#include "cluster.h"
#include "message.h"

namespace cr
{
    namespace app
    {
        class Application;

        /**
         * 表示一个服务
         */
        class Service : public std::enable_shared_from_this<Service>
        {
        public:

            /**
             * 构造函数
             * @param context app
             * @param ioService 工作线程
             * @param id 服务Id
             * @param name 服务名字
             */
            Service(Application& context, boost::asio::io_service& ioService, std::uint32_t id, std::string name);

            /**
             * 析构函数
             */
            virtual ~Service();

            /**
             * 获取应用上下文
             * @return app
             */
            Application& getApplicationContext();

            /**
             * 获取集群服务
             * @return 集群服务
             */
            const std::shared_ptr<Cluster> getCluster() const;

            /**
             * asio io_service
             * @return io_service
             */
            boost::asio::io_service& getIoService();

            /**
             * 获取服务Id
             * @param 服务Id
             */
            std::uint32_t getId() const;

            /**
             * 获取服务名字
             * @param 服务名字
             */
            const std::string& getName() const;

        protected:

            /**
             * 服务运行回调
             */
            virtual void onStart();

            /**
             * 服务结束回调
             */
            virtual void onStop();

            /**
             * 接受到一个本地消息
             * @param serviceId 源服务Id
             * @param session 会话Id
             * @param message 消息
             */
            virtual void onMessageReceived(std::uint32_t serviceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

            /**
             * 发送一个本地消息
             * @param serverId 目的服务Id
             * @param session 会话Id
             * @param message 消息
             */
            void sendMessage(std::uint32_t serverId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

        private:

            // 本地消息入队
            void onPushMessage(std::uint32_t serviceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

            // 弹出消息
            void popMessage();

            // 本地消息处理回调
            void onPopMessage(const boost::system::error_code& error);

            friend class Application;
            // 应用上下文
            Application& context_;
            // asio io_service
            boost::asio::io_service& ioService_;
            // Id
            std::uint32_t id_;
            // 名字
            std::string name_;
            // 消息队列
            using QueueMessage = std::tuple<std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>>;
            cr::concurrent::Pipe<QueueMessage, std::mutex> messageQueue_;
            std::vector<QueueMessage> popMessages_;
        };
    }
}

#endif