#include "application.h"

#include <algorithm>
#include <atomic>
#include <mutex>

#include <boost/core/null_deleter.hpp>

#include <cr/core/assert.h>

#include "local_cluster_impl.h"
#include "service.h"

namespace cr
{
    namespace app
    {

        Application::Application(boost::asio::io_service& ioService)
            : state_(NORMAL),
            ioService_(&ioService, boost::null_deleter()),
            cluster_(std::make_shared<LocalClusterImpl>(*ioService_)),
            nextId_(1),
            services_(16),
            collectionThread_(),
            collectionTimer_(ioService),
            mutexs_(services_.size())
        {
            // 主线程
            auto iter = workThreads_.emplace(std::make_pair("Main", std::make_pair(
                std::make_shared<cr::concurrent::Thread>(ioService_), std::set<std::uint32_t>())));
            iter.first->second.second.insert(0);
        }

        Application::~Application()
        {
            collectionThread_.stop();
            collectionThread_.join();
        }

        boost::asio::io_service& Application::getIoService()
        {
            return *ioService_;
        }

        void Application::onStart()
        {}

        void Application::onStop()
        {}

        void Application::onServiceStart(std::shared_ptr<Service> service)
        {}

        void Application::onServiceStop(std::shared_ptr<Service> service)
        {}

        void Application::stopService(std::uint32_t serviceId)
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            if (state_ == RUNNING)
            {
                auto service = getServiceNoGuard(serviceId);
                if (service != nullptr)
                {
                    stopServiceNoGuard(serviceId);
                }
            }
        }

        std::shared_ptr<Service> Application::getService(std::uint32_t serviceId) const
        {
            auto slot = serviceId % services_.size();
            std::lock_guard<std::mutex> locker(mutexs_.slot(slot));
            return getServiceNoGuard(serviceId);
        }

        std::vector<std::uint32_t> Application::getServiceIds() const
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            return getServiceIdsNoGuard();
        }

        std::vector<std::uint32_t> Application::getServiceIds(const std::string& name) const
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            auto serviceNameIter = names.find(name);
            if (serviceNameIter != names.end())
            {
                auto serviceIds = serviceNameIter->second;
                return { serviceIds.begin(), serviceIds.end() };
            }
            return {};
        }

        void Application::setCluster(std::shared_ptr<Cluster> cluster)
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            if (state_ == NORMAL)
            {
                CR_ASSERT(cluster != nullptr);
                cluster_ = std::move(cluster);
            }
        }

        const std::shared_ptr<Cluster>& Application::getCluster() const
        {
            return cluster_;
        }

        void Application::sendMessage(std::uint32_t fromServiceId, std::uint32_t toServiceId, std::uint64_t session, std::shared_ptr<google::protobuf::Message> message)
        {
            auto service = getService(toServiceId);
            if (service != nullptr)
            {
                service->onPushMessage(fromServiceId, session, std::move(message));
            }
        }

        void Application::sendMessage(std::uint32_t serviceId, std::shared_ptr<google::protobuf::Message> message)
        {
            sendMessage(0, serviceId, 0, std::move(message));
        }

        void Application::start()
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            if (state_ == NORMAL)
            {
                ioService_->post([this, self = shared_from_this()]
                {
                    work_ = std::make_unique<boost::asio::io_service::work>(*ioService_);
                    onStart();
                });
                startCollectionTimer();
                state_ = RUNNING;
            }
        }

        void Application::stop()
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            if (state_ == RUNNING)
            {
                auto self = shared_from_this();
                // 停止定时器
                collectionTimer_.cancel();
                //停止所有服务
                std::vector<std::uint32_t> serviceIds = getServiceIdsNoGuard();
                for (auto serviceId : serviceIds)
                {
                    stopServiceNoGuard(serviceId);
                }
                // 停止所有线程
                for (auto&& workThread : workThreads_)
                {
                    collectionThread(std::move(workThread.second.first));
                }
                workThreads_.clear();
                // 停止后台线程
                collectionThread_.post([this, self]
                {
                    ioService_->post([this, self] 
                    {
                        onStop();
                        work_.reset();
                    });
                });
                state_ = STOPED;
            }
        }

        std::future<std::uint32_t> Application::startService(std::string group, std::string name, ServiceBuilder builder)
        {
            std::uint32_t serviceId;
            std::shared_ptr<boost::asio::io_service> workIoService;
            {
                std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
                if (state_ == RUNNING)
                {
                    // 服务 id
                    serviceId = nextId_++;
                    // 分配服务空间
                    auto slot = serviceId % this->services_.size();
                    this->services_[slot].insert(std::make_pair(serviceId, std::shared_ptr<Service>()));
                    this->names[name].insert(serviceId);
                    // 线程组
                    workGroups_.insert(std::make_pair(serviceId, group));
                    // 工作线程
                    auto workThreadIter = workThreads_.find(group);
                    if (workThreadIter == workThreads_.end())
                    {
                        std::tie(workThreadIter, std::ignore) = workThreads_.emplace(std::make_pair(group,
                            std::make_pair(std::make_shared<cr::concurrent::Thread>(), std::set<std::uint32_t>())));
                    }
                    workThreadIter->second.second.insert(serviceId);
                    workIoService = workThreadIter->second.first->getIoService();
                }
                else
                {
                    serviceId = 0;
                }
            }
            // 在工作线程构造
            auto promise = std::make_shared<std::promise<std::uint32_t>>();
            if (serviceId != 0)
            {
                workIoService->dispatch([this, self = shared_from_this(), serviceId, workIoService, promise, builder, name]
                {
                    auto service = builder(*this, *workIoService, serviceId, name);
                    {
                        std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
                        if (this->state_ == RUNNING)
                        {
                            auto slot = serviceId % this->services_.size();
                            this->services_[slot][serviceId] = service;
                        }
                        else
                        {
                            service.reset();
                        }
                    }
                    if (service != nullptr)
                    {
                        service->onStart();
                        this->onServiceStart(service);
                        promise->set_value(serviceId);
                    }
                    else
                    {
                        promise->set_value(0);
                    }
                });
            }
            else
            {
                promise->set_value(0);
            }
            return promise->get_future();
        }

        void Application::stopServiceNoGuard(std::uint32_t serviceId)
        {
            auto slot = serviceId % services_.size();
            // service
            auto serviceIter = services_[slot].find(serviceId);
            CR_ASSERT(serviceIter != services_[slot].end())(serviceId);
            auto service = serviceIter->second;
            auto name = service->getName();
            services_[slot].erase(serviceIter);
            // 名字
            auto nameIter = names.find(name);
            CR_ASSERT(nameIter != names.end())(name);
            nameIter->second.erase(serviceId);
            if (nameIter->second.empty())
            {
                names.erase(nameIter);
            }
            // group 
            auto groupIter = workGroups_.find(serviceId);
            CR_ASSERT(groupIter != workGroups_.end())(serviceId);
            std::string group = groupIter->second;
            workGroups_.erase(groupIter);
            // work thread
            auto workThreadIter = workThreads_.find(group);
            CR_ASSERT(workThreadIter != workThreads_.end())(group);
            auto& workThreadEntry = workThreadIter->second;
            // 回调onStop
            workThreadEntry.first->post([this, self = shared_from_this(), service = std::move(service)]
            {
                if (service != nullptr)
                {
                    onServiceStop(service);
                    service->onStop();
                }
            });
            // 移除服务Id
            workThreadEntry.second.erase(serviceId);
        }

        std::shared_ptr<Service> Application::getServiceNoGuard(std::uint32_t serviceId) const
        {
            auto slot = serviceId % services_.size();
            auto serviceIter = services_[slot].find(serviceId);
            if (serviceIter != services_[slot].end())
            {
                return serviceIter->second;
            }
            return nullptr;
        }

        std::vector<std::uint32_t> Application::getServiceIdsNoGuard() const
        {
            std::vector<std::uint32_t> serviceIds;
            for (std::size_t i = 0; i != services_.size(); ++i)
            {
                for (auto& entry : services_[i])
                {
                    serviceIds.push_back(entry.first);
                }
            }
            return serviceIds;
        }

        void Application::startCollectionTimer()
        {
            collectionTimer_.expires_from_now(std::chrono::seconds(1));
            collectionTimer_.async_wait([this, self = shared_from_this()](const boost::system::error_code& error)
            {
                if (!error)
                {
                    onCollectionTimerHandler();
                    startCollectionTimer();
                }
            });
        }

        void Application::onCollectionTimerHandler()
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            if (state_ == RUNNING)
            {
                std::vector<std::string> gcWorkGroups;
                for (auto&& workThread : workThreads_)
                {
                    if (workThread.second.second.empty())
                    {
                        collectionThread(std::move(workThread.second.first));
                        gcWorkGroups.push_back(workThread.first);
                    }
                }
                for (auto&& group : gcWorkGroups)
                {
                    workThreads_.erase(group);
                }
            }
        }

        void Application::collectionThread(std::shared_ptr<cr::concurrent::Thread> thread)
        {
            auto promise = std::make_shared<std::promise<std::shared_ptr<cr::concurrent::Thread>>>();
            thread->post([thread, promise]() mutable
            {
                thread->stop();
                promise->set_value(std::move(thread));
                promise.reset();
            });
            collectionThread_.post([promise]() mutable
            {
                auto future = promise->get_future();
                auto thread = future.get();
                thread->join();
            });
        }
    }
}