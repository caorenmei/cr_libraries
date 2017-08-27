#ifndef CR_APP_APPLICATION_H_
#define CR_APP_APPLICATION_H_

#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

#include <cr/concurrent/multi_mutex.h>
#include <cr/concurrent/thread.h>

#include "cluster.h"
#include "message.h"

namespace cr
{
    namespace app
    {
        /** 服务 */
        class Service;

        /**
         * 表示一个应用
         */
        class Application : public std::enable_shared_from_this<Application>
        {
        public:

            /**
             * @param ioService io service
             */
            explicit Application(boost::asio::io_service& ioService);

            /**
             * 析构函数
             */
            virtual ~Application();

            Application(const Application&) = delete;
            Application& operator=(const Application&) = delete;

            /**
             * 获取主io_service
             */
            boost::asio::io_service& getIoService();

        protected:

            /**
             * 启动
             */
            virtual void onStart();

            /**
             * 停止
             */
            virtual void onStop();

            /**
             * 启动服务
             */
            virtual void onServiceStart(std::shared_ptr<Service> service);

            /**
             * 服务结束
             */
            virtual void onServiceStop(std::shared_ptr<Service> service);

        public:

            /**
             * 启动一个服务
             * @param threadGroup 线程组,主线程为Main
             * @param name 服务名字
             * @param args... 服务需要的参数
             */
            template <typename DerivedService, typename... Args>
            std::enable_if_t<std::is_base_of<Service, DerivedService>::value, std::future<std::uint32_t>> startService(std::string group, std::string name, Args&&... args)
            {
                auto builder = [](Application& context, boost::asio::io_service& ioService, std::uint32_t id, std::string name, auto&&... args)
                {
                    return std::make_shared<DerivedService>(context, ioService, id, name, std::forward<decltype(args)>(args)...);
                };
                return startService(group, name, std::bind(builder, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::forward<Args>(args)...));
            }

            /**
             * 停止一个服务
             * @param serviceId 服务Id
             */
            void stopService(std::uint32_t serviceId);

            /**
             * 获取一个服务
             * @param serviceId 服务Id
             */
            std::shared_ptr<Service> getService(std::uint32_t serviceId) const;

            /**
             * 获取所有服务Id
             * @reutrn 服务Id列表 
             */
            std::vector<std::uint32_t> getServiceIds() const;

            /**
             * 获取名字对应的所有服务Id
             * @param name 服务名字
             * @reutrn 服务Id列表 
             */
            std::vector<std::uint32_t> getServiceIds(const std::string& name) const;

            /**
             * 设置集群服务,须在start()之前调用
             * @apram cluster 集群服务
             */
            void setCluster(std::shared_ptr<Cluster> cluster);

            /**
             * 获取集群服务
             * @reutrn 集群服务
             */
            const std::shared_ptr<Cluster>& getCluster() const;

            /** 
             * 发送一个消息给本地服务
             * @param fromServiceId 源服务Id
             * @param toServiceId 目的服务Id
             * @param session 会话Id
             * @param message 消息
             */
            void sendMessage(std::uint32_t fromServiceId, std::uint32_t toServiceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message);

            /**
            * 发送一个消息给本地服务
            * @param serviceId 源服务Id
            * @param message 消息
            */
            void sendMessage(std::uint32_t serviceId, std::shared_ptr<google::protobuf::Message> message);

            /**
             * 开始运行
             */
            void start();

            /**
             * 终止
             */
            void stop();

        private:

            // 启动一个服务
            using ServiceBuilder = std::function<std::shared_ptr<Service>(Application&, boost::asio::io_service&, std::uint32_t, std::string)>;
            std::future<std::uint32_t> startService(std::string group, std::string name, ServiceBuilder builder);

            // 停止一个服务
            void stopServiceNoGuard(std::uint32_t serviceId);

            // 获取一个服务
            std::shared_ptr<Service> getServiceNoGuard(std::uint32_t serviceId) const;

            // 获取服务Id列表
            std::vector<std::uint32_t> getServiceIdsNoGuard() const;

            // 运行资源回收定时器
            void startCollectionTimer();

            // 资源回收处理器
            void onCollectionTimerHandler();

            // 回收线程
            void collectionThread(std::shared_ptr<cr::concurrent::Thread> thread);

            friend Service;
            // 状态
            enum State { NORMAL, RUNNING, STOPED, };
            State state_;
            // boost io_service
            std::shared_ptr<boost::asio::io_service> ioService_;
            std::unique_ptr<boost::asio::io_service::work> work_;
            // 集群服务
            std::shared_ptr<Cluster> cluster_;
            // 递增的服务Id
            std::uint32_t nextId_;
            // 服务列表
            std::vector<std::map<std::uint32_t, std::shared_ptr<Service>>> services_;
            std::map<std::string, std::set<std::uint32_t>> names;
            // 工作线程组
            std::map<std::string, std::pair<std::shared_ptr<cr::concurrent::Thread>, std::set<std::uint32_t>>> workThreads_;
            std::map<std::uint32_t, std::string> workGroups_;
            // background thread
            cr::concurrent::Thread collectionThread_;
            // 资源回收定时器
            boost::asio::steady_timer collectionTimer_;
            // 锁
            mutable cr::concurrent::MultiMutex<std::mutex> mutexs_;
        };
    }
}

#endif