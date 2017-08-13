#ifndef COMMON_SERVICE_H_
#define COMMON_SERVICE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

#include <boost/asio/io_service.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cr/concurrent/pipe.h>

#include "message.h"

namespace cr
{
    namespace app
    {
        class Application;

        // 表示一个服务
        class Service : public std::enable_shared_from_this<Service>
        {
        public:

            // @param app 应用上下文
            Service(Application& context, boost::asio::io_service& ioService, std::uint32_t id, const std::string& name);

            virtual ~Service();

            // 获取应用上下文
            Application& getApplicationContext();

            // asio io_service
            boost::asio::io_service& getIoService();

            // 服务Id
            std::uint32_t getId() const;

            // 服务名字
            const std::string& getName() const;

        protected:

            // 服务运行回调
            virtual void onStart();

            // 服务结束回调
            virtual void onStop();

            // 消息接收回调
            virtual void onMessageReceived(std::uint32_t sourceId, std::uint32_t sourceServiceId, std::uint64_t session, std::shared_ptr<Message> message) = 0;

        protected:

            // 发送消息
            void sendMessage(std::uint32_t destId, std::uint32_t destServiceId,
                std::uint64_t session, std::shared_ptr<Message> message);

            // 发送消息
            void sendMessage(std::uint32_t destServiceId, std::uint64_t session, std::shared_ptr<Message> message);

        private:

            // 消息入队
            void onServiceMessage(std::uint32_t sourceId, std::uint32_t sourceServiceId, std::uint64_t session, std::shared_ptr<Message> message);

            // 消息处理回调
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
            cr::concurrent::Pipe<std::tuple<std::uint32_t, std::uint32_t, std::uint64_t, std::shared_ptr<Message>>, std::mutex> messageQueue_;
            std::vector<std::tuple<std::uint32_t, std::uint32_t, std::uint64_t, std::shared_ptr<Message>>> popMessages_;
        };
    }
}

#endif