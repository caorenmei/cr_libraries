#include "local_cluster_impl.h"

#include <cassert>
#include <chrono>

#include "cluster_proxy_impl.h"

namespace cr
{
    namespace app
    {
        LocalClusterImpl::LocalClusterImpl(boost::asio::io_service& ioService)
            : ioService_(ioService),
            uniqueId_(0),
            nextId_(1)
        {}

        LocalClusterImpl::~LocalClusterImpl()
        {}

        std::shared_ptr<ClusterProxy> LocalClusterImpl::connect(boost::asio::io_service& ioService, std::string name, std::string data)
        {
            std::uint32_t fromId = nextId_++;
            auto proxy = std::make_shared<ClusterProxyImpl>(ioService_, ioService, name);
            // 设置回调
            std::weak_ptr<void> self = shared_from_this();
            proxy->setClusterDisconnectHander(ioService_.wrap([this, self, fromId]
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyDisconnect(fromId);
                }
            }));
            proxy->setClusterWatchHander(ioService_.wrap([this, self, fromId](std::string name)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyWatch(fromId, std::move(name));
                }
            }));
            proxy->setClusterUnWatchHander(ioService_.wrap([this, self, fromId](std::string name)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyUnWatch(fromId, std::move(name));
                }
            }));
            proxy->setClusterDataUpdateHander(ioService_.wrap([this, self, fromId](std::string data)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyDataUpdate(fromId, std::move(data));
                }
            }));
            // 回调连接建立
            ioService_.post([this, self = shared_from_this(), fromId, proxy, name = std::move(name), data = std::move(data)]
            {
                onProxyConnect(fromId, proxy, std::move(name), std::move(data));
            });
            return proxy;
        }

        std::uint64_t LocalClusterImpl::genUuid()
        {
            // 时间戳从2000-01-01 00:00:00开始计算，可以支持到2069年
            constexpr std::uint64_t epochTime = 946684800000ull;
            while (true)
            {
                std::uint64_t nowTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() - epochTime;
                std::uint64_t prevUniqueId = uniqueId_.load(std::memory_order_relaxed);
                std::uint64_t seqnenceId = prevUniqueId & 0xfffull;
                std::uint64_t prevTs = (prevUniqueId >> 22) & 0x1ffffffffffull;
                // 序列号已分配完毕，等待
                if (prevTs == nowTime && seqnenceId == 0xfffull)
                {
                    continue;
                }
                seqnenceId = (prevTs == nowTime) ? seqnenceId + 1 : 0;
                std::uint64_t nowUniqueId = ((nowTime & 0x1ffffffffffull) << 22) | seqnenceId;
                if (uniqueId_.compare_exchange_strong(prevUniqueId, nowUniqueId, std::memory_order_relaxed))
                {
                    return nowUniqueId;
                }
            }
        }

        bool LocalClusterImpl::isValid() const
        {
            return true;
        }

        void LocalClusterImpl::onProxyConnect(std::uint32_t fromId, std::shared_ptr<ClusterProxyImpl> proxy, std::string name, std::string data)
        {
            std::weak_ptr<void> self = shared_from_this();
            // 设置回调
            proxy->setClusterMessageHander([this, self, fromId](std::uint32_t toId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
            {
                auto p = self.lock();
                if (p)
                {
                    onProxyMessage(fromId, toId, session, std::move(message));
                }
            });
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
            // proxy
            auto proxyIter = proxies_.find(fromId);
            assert(proxyIter != proxies_.end());
            proxyIter->second->setClusterMessageHander(nullptr);
            proxies_.erase(proxyIter);
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
                fromProxyIter->second->onClusterWatchEvent(ClusterProxy::ADD, name, proxyId, dataIter->second);
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