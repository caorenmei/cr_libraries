#include "service.h"

#include <functional>

#include "application.h"

namespace cr
{
    namespace app
    {
        Service::Service(Application& context, boost::asio::io_service& ioService, std::uint32_t id, const std::string& name)
            : context_(context),
            ioService_(ioService),
            id_(id),
            name_(name),
            messageQueue_(ioService_)
        {}

        Service::~Service()
        {}

        Application& Service::getApplicationContext()
        {
            return context_;
        }

        boost::asio::io_service& Service::getIoService()
        {
            return ioService_;
        }

        const std::string& Service::getName() const
        {
            return name_;
        }

        std::uint32_t Service::getId() const
        {
            return id_;
        }

        void Service::onStart()
        {
            messageQueue_.pop(popMessages_, std::bind(&Service::onPopMessage, shared_from_this(), std::placeholders::_1));
        }

        void Service::onStop()
        {
            messageQueue_.cancel();
        }

        void Service::sendMessage(std::uint32_t destId, std::uint32_t destServiceId,
            std::uint64_t session, std::shared_ptr<Message> message)
        {
            const auto& cluster = context_.getCluster();
            CR_ASSERT(cluster != nullptr);
            cluster->dispatchMessage(0, id_, destId, destServiceId, session, std::move(message));
        }

        void Service::sendMessage(std::uint32_t destServiceId, std::uint64_t session, std::shared_ptr<Message> message)
        {
            const auto& cluster = context_.getCluster();
            CR_ASSERT(cluster != nullptr);
            cluster->dispatchMessage(0, id_, 0, destServiceId, session, std::move(message));
        }

        void Service::onServiceMessage(std::uint32_t sourceId, std::uint32_t sourceServiceId, std::uint64_t session, std::shared_ptr<Message> message)
        {
            messageQueue_.push(std::make_tuple(sourceId, sourceServiceId, session, std::move(message)));
        }

        void Service::onPopMessage(const boost::system::error_code& error)
        {
            if (!error)
            {
                for (auto& message : popMessages_)
                {
                    onMessageReceived(std::get<0>(message), std::get<1>(message), std::get<2>(message), std::move(std::get<3>(message)));
                }
                popMessages_.clear();
                messageQueue_.pop(popMessages_, std::bind(&Service::onPopMessage, shared_from_this(), std::placeholders::_1));
            }
        }
    }
}
