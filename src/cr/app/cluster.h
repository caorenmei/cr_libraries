#ifndef COMMON_APP_CLUSTER_H_
#define COMMON_APP_CLUSTER_H_

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "message.h"

namespace cr
{
    namespace app
    {
        // 集群接口
        class Cluster
        {
        public:

            virtual ~Cluster() {}

            // 注册集群可见服务
            virtual void registerService(std::string name, std::uint32_t serviceId) = 0;

            // 取消注册服务
            virtual void unregisterService(std::uint32_t serviceId) = 0;

            // 获取本机Id
            virtual std::pair<std::uint32_t, bool> getId(std::function<void()> watcher) = 0;

            // 拥有该名字的主机 Id 列表
            virtual std::vector<std::uint32_t> getNames(const std::string& name, std::function<void()> watcher) = 0;

            // 获取服务一台服务器上的服务列表
            virtual std::pair<std::vector<uint32_t>, bool> getServices(const std::string& name, std::uint32_t hostId, std::function<void()> watcher) = 0;

            // 注册消息分发器, arg0: 来源Id, arg1: 来源服务Id, arg2: 目的Id, arg3: 目的服务Id, arg4: session, arg5: 消息
            virtual void setMessageDispatcher(
                std::function<void(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint64_t, std::shared_ptr<Message>) > dispatcher) = 0;

            // 分发消息
            virtual void dispatchMessage(std::uint32_t sourceId, std::uint32_t fromServieId,
                std::uint32_t destId, std::uint32_t destServiceId,
                std::uint64_t session, std::shared_ptr<Message> message) = 0;
        };
    }
}

#endif
