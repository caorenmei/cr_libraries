#ifndef COMMON_APP_LOCAL_CLUSTER_H_
#define COMMON_APP_LOCAL_CLUSTER_H_

#include <map>
#include <memory>
#include <mutex>
#include <set>

#include <boost/asio/io_service.hpp>

#include "cluster.h"

namespace cr
{
    namespace app
    {
        // 集群接口
        class LocalCluster : public std::shared_ptr<LocalCluster>, public Cluster
        {
        public:

            explicit LocalCluster(boost::asio::io_service& ioService);

            virtual ~LocalCluster() override;

            // 注册服务
            virtual void registerService(std::string name, std::uint32_t serviceId) override;

            // 取消服务注册
            virtual void unregisterService(std::uint32_t serviceId) override;

            // 获取自己的Id
            virtual std::pair<std::uint32_t, bool> getId(std::function<void()> watcher) override;

            // 获取主机列表
            virtual std::vector<std::uint32_t> getNames(const std::string& name, std::function<void()> watcher) override;

            // 获取服务列表
            virtual std::pair<std::vector<uint32_t>, bool> getServices(const std::string& name, std::uint32_t hostId, std::function<void()> watcher) override;

            // 注册消息分发器
            virtual void setMessageDispatcher(
                std::function<void(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
                    std::uint64_t, std::shared_ptr<google::protobuf::Message>)> dispatcher) override;

            // 分发消息
            virtual void dispatchMessage(std::uint32_t sourceId, std::uint32_t fromServieId,
                std::uint32_t destId, std::uint32_t destServiceId, 
                std::uint64_t session, std::shared_ptr<google::protobuf::Message> message) override;

        private:

            boost::asio::io_service& ioService_;
            // 自己的Id
            std::uint32_t myId_;
            // 服务列表
            std::map<std::string, std::set<std::uint32_t>> serviceIds_;
            std::map<std::uint32_t, std::string> nameServiceIds_;
            // 消息分发器
            std::function<void(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
                std::uint64_t, std::shared_ptr<google::protobuf::Message>)> dispatcher_;
            // 服务监视器
            std::map<std::string, std::vector<std::function<void()>>> nameWatchers_;
            // 服务监视器
            std::map<std::pair<std::string, std::uint32_t>, std::vector<std::function<void()>>> serviceWatchers_;
            // 锁
            std::mutex mutex_;
        };
    }
}

#endif
