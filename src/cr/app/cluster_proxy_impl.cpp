#include "cluster_proxy_impl.h"

namespace cr
{
    namespace app
    {
        ClusterProxyImpl::ClusterProxyImpl(boost::asio::io_service& clusterIoService, boost::asio::io_service& proxyIoService, std::string name)
            : clusterIoService_(clusterIoService),
            proxyIoService_(proxyIoService),
            state_(NORMAL),
            name_(std::move(name)),
            id_(0),
            clusterMessageQueue_(clusterIoService_),
            proxyMessageQueue_(proxyIoService_)
        {}

        ClusterProxyImpl::~ClusterProxyImpl()
        {}

        const std::string& ClusterProxyImpl::getName() const
        {
            return name_;
        }

        std::uint32_t ClusterProxyImpl::getId() const
        {
            return id_;
        }

        void ClusterProxyImpl::disconnect()
        {
            if (state_ != DISCONNECTED)
            {
                clusterMessageQueue_.interrupt();
                proxyMessageQueue_.interrupt();
                if (clusterDisconnectHandler_)
                {
                    clusterDisconnectHandler_();
                }
                state_ = DISCONNECTED;
                clusterDisconnectHandler_ = nullptr;
                clusterWatchHander_ = nullptr;
                clusterUnWatchHander_ = nullptr;
                clusterDataHandler_ = nullptr;
                // clusterMessageHandler_ = nullptr;
                proxyConnectHandler_ = nullptr;
                proxyWatchHandlers_.clear();
                proxyMessageHandler_ = nullptr;
            }
        }

        void ClusterProxyImpl::onConnect(std::function<void(std::uint32_t)> handler)
        {
            if (state_ != DISCONNECTED)
            {
                proxyConnectHandler_ = std::move(handler);
            }
        }

        void ClusterProxyImpl::watch(std::string name, std::function<void(Event, std::string, std::uint32_t, std::string)> handler)
        {
            if (state_ == CONNECTED)
            {
                proxyWatchHandlers_[name] = std::move(handler);
                clusterWatchHander_(std::move(name));
            }
        }

        void ClusterProxyImpl::unwatch(std::string name)
        {
            if (state_ == CONNECTED)
            {
                clusterUnWatchHander_(std::move(name));
            }
        }

        void ClusterProxyImpl::update(std::string data)
        {
            if (state_ == CONNECTED)
            {
                clusterDataHandler_(std::move(data));
            }
        }

        void ClusterProxyImpl::onMessageReceived(std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> handler)
        {
            if (state_ != DISCONNECTED)
            {
                proxyMessageHandler_ = std::move(handler);
            }
        }

        void ClusterProxyImpl::sendMessage(std::uint32_t id, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            if (state_ == CONNECTED)
            {
                clusterMessageQueue_.emplace(id, session, std::move(message));
            }
        }

        void ClusterProxyImpl::onClusterConnect(std::uint32_t id)
        {
            proxyIoService_.post([this, self = shared_from_this(), id]
            {
                onProxyConnect(id);
            });
        }

        void ClusterProxyImpl::onClusterMessageReceive(std::uint32_t id, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            proxyMessageQueue_.emplace(id, session, std::move(message));
        }

        void ClusterProxyImpl::onClusterWatchEvent(Event event, std::string name, std::uint32_t fromId, std::string data)
        {
            onProxyWatchEvent(event, name, fromId, std::move(data));
        }

        void ClusterProxyImpl::setClusterDisconnectHander(std::function<void()> handler)
        {
            onSetClusterDisconnectHander(std::move(handler));
        }

        void ClusterProxyImpl::setClusterWatchHander(std::function<void(std::string)> handler)
        {
            if (state_ != DISCONNECTED)
            {
                clusterWatchHander_ = std::move(handler);
            }
        }

        void ClusterProxyImpl::setClusterUnWatchHander(std::function<void(std::string)> handler)
        {
            if (state_ != DISCONNECTED)
            {
                clusterUnWatchHander_ = std::move(handler);
            }
        }

        void ClusterProxyImpl::setClusterDataUpdateHander(std::function<void(std::string)> handler)
        {
            if (state_ != DISCONNECTED)
            {
                clusterDataHandler_ = std::move(handler);
            }
        }

        void ClusterProxyImpl::setClusterMessageHander(std::function<void(std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>)> handler)
        {
            clusterMessageHandler_ = std::move(handler);
        }

        void ClusterProxyImpl::onProxyConnect(std::uint32_t id)
        {
            id_ = id;
            if (state_ == NORMAL)
            {
                state_ = CONNECTED;
                // 连接状态
                if (proxyConnectHandler_)
                {
                    proxyConnectHandler_(id_);
                }
                // 开始消息队列
                popClusterMessage();
                popProxyMessage();
            }
        }

        void ClusterProxyImpl::onSetClusterDisconnectHander(std::function<void()> handler)
        {
            clusterDisconnectHandler_ = std::move(handler);
            if (state_ == DISCONNECTED)
            {
                clusterDisconnectHandler_();
                clusterDisconnectHandler_ = nullptr;
            }
        }

        void ClusterProxyImpl::onProxyWatchEvent(Event event, std::string name, std::uint32_t fromId, std::string data)
        {
            if (state_ == CONNECTED)
            {
                auto handlerIter = proxyWatchHandlers_.find(name);
                if (handlerIter != proxyWatchHandlers_.end())
                {
                    handlerIter->second(event, name, fromId, std::move(data));
                }
            }
        }

        void ClusterProxyImpl::popClusterMessage()
        {
            clusterMessageQueue_.pop(popClusterMessages_, [this, self = shared_from_this()](const boost::system::error_code& error, std::size_t)
            {
                if (!error)
                {
                    onPopClusterMessage();
                    popClusterMessage();
                }
            });
        }

        void ClusterProxyImpl::onPopClusterMessage()
        {
            for (auto&& message : popClusterMessages_)
            {
                if (clusterMessageHandler_ != nullptr)
                {
                    clusterMessageHandler_(std::get<0>(message), std::get<1>(message), std::move(std::get<2>(message)));
                }
            }
            popClusterMessages_.clear();
        }

        void ClusterProxyImpl::popProxyMessage()
        {
            proxyMessageQueue_.pop(popProxyMessages_, [this, self = shared_from_this()](const boost::system::error_code& error, std::size_t)
            {
                if (!error)
                {
                    onPopProxyMessage();
                    popProxyMessage();
                }
            });
        }

        void ClusterProxyImpl::onPopProxyMessage()
        {
            for (auto&& message : popProxyMessages_)
            {
                if (proxyMessageHandler_ != nullptr)
                {
                    proxyMessageHandler_(std::get<0>(message), std::get<1>(message), std::move(std::get<2>(message)));
                }
            }
            popProxyMessages_.clear();
        }
    }
}