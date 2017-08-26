#include "service.h"

#include <functional>

#include <cr/common/assert.h>

#include "application.h"

namespace cr
{
    namespace app
    {

        Service::Service(Application& context, boost::asio::io_service& ioService, std::uint32_t id, std::string name)
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

        const std::shared_ptr<Cluster> Service::getCluster() const
        {
            return context_.getCluster();
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
            popMessage();
        }

        void Service::onStop()
        {
            messageQueue_.interrupt();
        }

        void Service::onMessageReceived(std::uint32_t serviceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {}

        void Service::sendMessage(std::uint32_t serverId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            context_.sendMessage(id_, serverId, session, std::move(message));
        }

        void Service::onPushMessage(std::uint32_t serviceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            messageQueue_.emplace(serviceId, session, std::move(message));
        }

        void Service::popMessage()
        {
            std::weak_ptr<void> self = shared_from_this();
            messageQueue_.pop(popMessages_, [this, self](const boost::system::error_code& error, std::size_t)
            {
                auto p = self.lock();
                if (p)
                {
                    onPopMessage(error);
                }
            });
        }

        void Service::onPopMessage(const boost::system::error_code& error)
        {
            if (!error)
            {
                for (auto& message : popMessages_)
                {
                    onMessageReceived(std::get<0>(message), std::get<1>(message), std::move(std::get<2>(message)));
                }
                popMessages_.clear();
                popMessage();
            }
        }
    }
}
