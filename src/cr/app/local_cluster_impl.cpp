﻿#include "local_cluster_impl.h"

#include <cassert>

#include "cluster_proxy_impl.h"

namespace cr
{
    namespace app
    {
        LocalClusterImpl::LocalClusterImpl(boost::asio::io_service& ioService)
            : ioService_(ioService),
            nextId_(1)
        {}

        LocalClusterImpl::~LocalClusterImpl()
        {}

        std::shared_ptr<ClusterProxy> LocalClusterImpl::connect(boost::asio::io_service& ioService, std::string name, std::string data)
        {
            std::weak_ptr<void> self = shared_from_this();
            std::uint32_t id = nextId_++;
            auto proxy = std::make_shared<ClusterProxyImpl>(ioService_, ioService, name);
            proxy->setClusterDisconnectHander(ioService_.wrap([this, self, id]
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyDisconnect(id);
                }
            }));
            proxy->setClusterWatchHander(ioService_.wrap([this, self, id](std::string name)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyWatch(id, std::move(name));
                }
            }));
            proxy->setClusterUnWatchHander(ioService_.wrap([this, self, id](std::string name)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyUnWatch(id, std::move(name));
                }
            }));
            proxy->setClusterDataUpdateHander(ioService_.wrap([this, self, id](std::string data)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyDataUpdate(id, std::move(data));
                }
            }));
            proxy->setClusterMessageHander([this, self, id](std::uint32_t toId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyMessage(id, toId, session, std::move(message));
                }
            });
            ioService_.post([this, self = shared_from_this(), id, proxy, name = std::move(name), data = std::move(data)]
            {
                onProxyConnect(id, proxy, std::move(name), std::move(data));
            });
            return proxy;
        }

        void LocalClusterImpl::onProxyConnect(std::uint32_t fromId, std::shared_ptr<ClusterProxyImpl> proxy, std::string name, std::string data)
        {
            // 保存数据
            nameIds_[name].insert(fromId);
            idNames_.insert(std::make_pair(fromId, name));
            datas_.insert(std::make_pair(fromId, data));
            proxies_.insert(std::make_pair(fromId, proxy));
            // 服务建立成功
            proxy->onClusterConnect(fromId);
            // 通知其它服务
            for (auto toId : watchNameIds_[name])
            {
                auto proxyIter = proxies_.find(toId);
                assert(proxyIter != proxies_.end());
                proxyIter->second->onClusterWatchEvent(ClusterProxy::ADD, name, fromId, data);
            }
        }

        void LocalClusterImpl::onProxyDisconnect(std::uint32_t fromId)
        {
            // 清除数据结构
            // 名字
            auto nameIter = idNames_.find(fromId);
            assert(nameIter != idNames_.end());
            auto name = nameIter->second;
            idNames_.erase(nameIter);
            nameIds_[name].erase(fromId);
            // proxy
            auto proxyIter = proxies_.find(fromId);
            assert(proxyIter != proxies_.end());
            proxies_.erase(proxyIter);
            // data
            auto dataIter = datas_.find(fromId);
            assert(dataIter != datas_.end());
            datas_.erase(dataIter);
            // watch
            for (auto watchName : watchIdNames_[fromId])
            {
                watchNameIds_[watchName].erase(fromId);
            }
            watchIdNames_.erase(fromId);
            // 通知其它服务
            for (auto toId : watchNameIds_[name])
            {
                auto proxyIter = proxies_.find(toId);
                assert(proxyIter != proxies_.end());
                proxyIter->second->onClusterWatchEvent(ClusterProxy::REMOVE, name, fromId, "");
            }
        }

        void LocalClusterImpl::onProxyMessage(std::uint32_t fromId, std::uint32_t toId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            auto proxyIter = proxies_.find(toId);
            if (proxyIter != proxies_.end())
            {
                proxyIter->second->onClusterMessageReceive(fromId, session, std::move(message));
            }
        }

        void LocalClusterImpl::onProxyWatch(std::uint32_t fromId, std::string name)
        {
            auto fromProxyIter = proxies_.find(fromId);
            assert(fromProxyIter != proxies_.end());
            // 先通知一遍
            for (auto proxyId : nameIds_[name])
            {
                auto dataIter = datas_.find(proxyId);
                assert(dataIter != datas_.end());
                fromProxyIter->second->onClusterWatchEvent(ClusterProxy::UPDATE, name, proxyId, dataIter->second);
            }
            // 保存
            watchNameIds_[name].insert(fromId);
            watchIdNames_[fromId].insert(name);
        }

        void LocalClusterImpl::onProxyUnWatch(std::uint32_t fromId, std::string name)
        {
            for (auto watchName : watchIdNames_[fromId])
            {
                watchNameIds_[watchName].erase(fromId);
            }
            watchIdNames_.erase(fromId);
        }

        void LocalClusterImpl::onProxyDataUpdate(std::uint32_t fromId, std::string data)
        {
            auto nameIter = idNames_.find(fromId);
            assert(nameIter != idNames_.end());
            auto& name = nameIter->second;
            // 保存数据
            auto dataIter = datas_.find(fromId);
            assert(dataIter != datas_.end());
            dataIter->second = data;
            // 通知
            for (auto toId : watchNameIds_[name])
            {
                auto proxyIter = proxies_.find(toId);
                assert(proxyIter != proxies_.end());
                proxyIter->second->onClusterWatchEvent(ClusterProxy::UPDATE, name, fromId, name);
            }
        }
    }
}