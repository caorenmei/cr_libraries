#ifndef COMMON_APP_APPLICATION_H_
#define COMMON_APP_APPLICATION_H_

#include <cstdint>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/io_service.hpp>

#include <cr/common/logging.h>

#include "cluster.h"
#include "message.h"

namespace cr
{
    namespace app
    {
        // 服务
        class Service;

        // 表示一个应用
        class Application : public std::enable_shared_from_this<Application>
        {
        public:

            Application(boost::asio::io_service& ioService, std::string name);

            virtual ~Application();

            Application(const Application&) = delete;
            Application& operator=(const Application&) = delete;

            // 获取主io_service
            boost::asio::io_service& getIoService();

        protected:

            // 启动
            virtual void onStart();

            // 停止
            virtual void onStop();

            // 启动服务
            virtual void onServiceStart(std::shared_ptr<Service> service);

            // 服务结束
            virtual void onServiceStop(std::shared_ptr<Service> service);

        public:

            // 启动一个服务
            template <typename DerivedService, typename... Args>
            std::enable_if_t<std::is_base_of<Service, DerivedService>::value, std::future<std::uint32_t>> 
                startService(const std::string& threadGroup, Args&&... args)
            {
                namespace holders = std::placeholders;
                auto factory = [](Application& context, boost::asio::io_service& ioService, std::uint32_t id, Args&&... args)
                {
                    return std::make_shared<DerivedService>(context, ioService, id, std::forward<Args>(args)...);
                }; 
                return startService(threadGroup, std::bind(factory, holders::_1, holders::_2, holders::_3, std::forward<Args>(args)...));
            }

            // 启动一个服务
            std::future<std::uint32_t> startService(const std::string& threadGroup, 
                std::function<std::shared_ptr<Service>(Application&, boost::asio::io_service&, std::uint32_t)> factory);

            // 停止一个服务
            void stopService(std::uint32_t serviceId);

            // 获取一个服务
            std::shared_ptr<Service> getService(std::uint32_t serviceId) const;

            // 获取服务Id列表
            std::vector<std::uint32_t> getServiceIds() const;

            // 获取服务名字列表
            std::vector<std::string> getServiceNames() const;

            // 查找名字对应的所有服务Id
            std::vector<std::uint32_t> findServiceIds(const std::string& name) const;

            // 设置集群服务
            void setCluster(std::shared_ptr<Cluster> cluster);

            // 获取集群服务
            const std::shared_ptr<Cluster>& getCluster() const;

            // 开始运行
            void start();

            // 终止
            void stop();

        private:

            friend Service;

            // 派发服务间的消息
            void dispatchMessage(std::uint32_t sourceId, std::uint32_t sourceServiceId, 
                std::uint32_t destId, std::uint32_t destServiceId, 
                std::uint64_t session, std::shared_ptr<Message> message);

            // 停止一个服务
            void stopServiceNoGuard(std::uint32_t serviceId);

            // boost io_service
            std::shared_ptr<boost::asio::io_service> ioService_;
            std::unique_ptr<boost::asio::io_service::work> work_;
            // 日志
            cr::log::Logger logger_;
            // 集群服务
            std::shared_ptr<Cluster> cluster_;
            // 递增的服务Id
            std::uint32_t nextServiceId_;
            // 服务列表
            std::vector<std::map<std::uint32_t, std::shared_ptr<Service>>> services_;
            // 服务Id列表
            std::list<std::uint32_t> serviceIds_;
            // 退役服务列表
            std::set<std::uint32_t> stopServiceIds_;
            // 名字-服务Id映射表
            std::map<std::string, std::set<std::uint32_t>> serviceNames_;
            // 服务对应的线程组
            std::map<std::uint32_t, std::string> threadGroups_;
            // 线程组
            std::map<std::string, std::pair<std::shared_ptr<boost::asio::io_service>, std::size_t>> threads_;
            // 锁
            mutable std::vector<std::unique_ptr<std::mutex>> mutexs_;
        };
    }
}

#endif