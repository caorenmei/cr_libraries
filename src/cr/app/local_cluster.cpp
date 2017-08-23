#include "local_cluster.h"

#include <cr/common/assert.h>

namespace cr
{
    namespace app
    {

        LocalCluster::LocalCluster(boost::asio::io_service& ioService)
            : ioService_(ioService),
            myId_(1)
        {}

        LocalCluster::~LocalCluster()
        {}

        void LocalCluster::registerService(std::string name, std::uint32_t serviceId)
        {
            std::lock_guard<std::mutex> locker(mutex_);
            auto iter = serviceIds_.find(name);
            if (iter == serviceIds_.end())
            {
                iter = serviceIds_.insert(std::make_pair(name, std::set<std::uint32_t>())).first;
                // 通知服务名字监听
                std::vector<std::uint32_t> nameIds = { myId_ };
                for (auto&& watcher : nameWatchers_[name])
                {
                    ioService_.post(std::bind(std::move(watcher)));
                }
                nameWatchers_.erase(name);
            }
            iter->second.insert(serviceId);
            nameServiceIds_[serviceId] = name;
            // 通知服务列表变更
            std::vector<std::uint32_t> serviceIds = { iter->second.begin(), iter->second.end() };
            for (auto&& watcher : serviceWatchers_[std::make_pair(name, myId_)])
            {
                ioService_.post(std::bind(std::move(watcher)));
            }
            serviceWatchers_.erase(std::make_pair(name, myId_));
        }

        void LocalCluster::unregisterService(std::uint32_t serviceId)
        {
            std::lock_guard<std::mutex> locker(mutex_);
            auto iter = nameServiceIds_.find(serviceId);
            if (iter != nameServiceIds_.end())
            {
                std::string name = iter->second;
                serviceIds_[name].erase(serviceId);
                // 通知服务列表变更
                std::vector<std::uint32_t> serviceIds = { serviceIds_[name].begin(), serviceIds_[name].end() };
                for (auto&& watcher : serviceWatchers_[std::make_pair(name, myId_)])
                {
                    ioService_.post(std::bind(std::move(watcher)));
                }
                serviceWatchers_.erase(std::make_pair(name, myId_));
                // 通知服务名字变化
                if (serviceIds_[name].empty())
                {
                    std::vector<std::uint32_t> nameIds = { myId_ };
                    for (auto&& watcher : nameWatchers_[name])
                    {
                        ioService_.post(std::bind(std::move(watcher)));
                    }
                    nameWatchers_.erase(name);
                    serviceIds_.erase(name);
                }
            }
        }

        std::pair<std::uint32_t, bool> LocalCluster::getId(std::function<void()> watcher)
        {
            return std::make_pair(myId_, true);
        }

        std::vector<std::uint32_t> LocalCluster::getNames(const std::string& name, std::function<void()> watcher)
        {
            std::lock_guard<std::mutex> locker(mutex_);
            nameWatchers_[name].push_back(std::move(watcher));
            auto nameIter = serviceIds_.find(name);
            if (nameIter != serviceIds_.end())
            {
                return { myId_ };
            }
            return {};
        }

        std::pair<std::vector<uint32_t>, bool> LocalCluster::getServices(const std::string& name, std::uint32_t hostId, std::function<void()> watcher)
        {
            std::lock_guard<std::mutex> locker(mutex_);
            if (hostId != myId_)
            {
                return {};
            }
            std::vector<std::uint32_t> serviceIds;
            auto nameIter = serviceIds_.find(name);
            if (nameIter != serviceIds_.end())
            {
                serviceIds = { nameIter->second.begin(), nameIter->second.end() };
                serviceWatchers_[std::make_pair(name, hostId)].push_back(std::move(watcher));
                return std::make_pair(serviceIds, true);
            }
            return std::make_pair(serviceIds, false);
        }

        void LocalCluster::setMessageDispatcher(
            std::function<void(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint64_t, std::shared_ptr<google::protobuf::Message>) > dispatcher)
        {
            dispatcher_ = std::move(dispatcher);
        }

        void LocalCluster::dispatchMessage(std::uint32_t sourceId, std::uint32_t fromServieId, 
            std::uint32_t destId, std::uint32_t destServiceId, 
            std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            CR_ASSERT(dispatcher_ != nullptr);
            if (destId == myId_)
            {
                destId = 0;
            }
            if (destId == 0)
            {
                dispatcher_(sourceId, fromServieId, destId, destServiceId, session, std::move(message));
            }
        }
    }
}