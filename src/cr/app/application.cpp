#include "application.h"

#include <algorithm>
#include <atomic>
#include <mutex>

#include <boost/core/null_deleter.hpp>

#include <cr/common/assert.h>

#include "local_cluster.h"
#include "service.h"

namespace cr
{
    namespace app
    {

        Application::Application(boost::asio::io_service& ioService)
            : ioService_(&ioService, boost::null_deleter()),
            cluster_(std::make_shared<LocalCluster>(*ioService_)),
            nextId_(1),
            services_(16),
            backgroundThread_(),
            mutexs_(services_.size())
        {
            auto iter = workThreads_.emplace(std::make_pair("Main", std::make_pair(
                cr::concurrent::Thread(ioService_), std::set<std::uint32_t>())));
            iter.first->second.second.insert(0);
        }

        Application::~Application()
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            for (auto& workThread : workThreads_)
            {
                if (workThread.second.first.getThreadNum() != 0)
                {
                    workThread.second.first.post([&workThread]
                    {
                        workThread.second.first.stop();
                    });
                    workThread.second.first.join();
                }
            }
            backgroundThread_.post([this]
            {
                backgroundThread_.stop();
            });
            backgroundThread_.join();
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
            stopServiceNoGuard(serviceId, [this](std::shared_ptr<Service> service)
            {
                this->onServiceStop(service);
            });
        }

        std::shared_ptr<Service> Application::getService(std::uint32_t serviceId) const
        {
            auto slot = serviceId % services_.size();
            std::lock_guard<std::mutex> locker(mutexs_.slot(slot));
            auto serviceIter = services_[slot].find(serviceId);
            if (serviceIter != services_[slot].end())
            {
                return serviceIter->second;
            }
            return nullptr;
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
            CR_ASSERT(cluster != nullptr);
            cluster_ = std::move(cluster);
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

        void Application::runInBackground(std::function<void()> handler)
        {
            backgroundThread_.post(std::move(handler));
        }

        void Application::start()
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            CR_ASSERT(work_ == nullptr);
            ioService_->post([this, self = shared_from_this()]
            {
                onStart();
            });
            work_ = std::make_unique<boost::asio::io_service::work>(*ioService_);
        }

        void Application::stop()
        {
            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
            CR_ASSERT(work_ != nullptr);
            std::vector<std::uint32_t> serviceIds = getServiceIdsNoGuard();
            auto self = shared_from_this();
            // 所有服务销毁，调用onStop
            auto count = std::make_shared<std::atomic<std::size_t>>(serviceIds.size() + 1);
            auto countDown = [this, self, count]
            {
                if (--*count == 0)
                {
                    ioService_->post([this, self]
                    {
                        bool handle = false;
                        {
                            std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
                            if (work_ != nullptr)
                            {
                                handle = true;
                                work_.reset();
                            } 
                        }
                        if (handle)
                        {
                            onStop();
                        }
                    });
                }
            };
            //停止所有服务
            for (auto serviceId : serviceIds)
            {
                stopServiceNoGuard(serviceId, [this, self, countDown](std::shared_ptr<Service> service)
                {
                    onServiceStop(service);
                    countDown();
                });
            }
            countDown();
        }

        std::future<std::uint32_t> Application::startService(std::string group, std::string name, ServiceBuilder builder)
        {
            std::uint32_t serviceId;
            std::shared_ptr<boost::asio::io_service> workIoService;
            {
                std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
                // 服务 id
                serviceId = nextId_++;
                // 线程组
                workGroups_.insert(std::make_pair(serviceId, group));
                // 工作线程
                auto workThreadIter = workThreads_.find(group);
                if (workThreadIter == workThreads_.end())
                {
                    std::tie(workThreadIter, std::ignore) = workThreads_.emplace(std::make_pair(group, 
                        std::make_pair(cr::concurrent::Thread(), std::set<std::uint32_t>())));
                }
                workThreadIter->second.second.insert(serviceId);
                workIoService = workThreadIter->second.first.getIoService();
            }
            // 在工作线程构造
            auto promise = std::make_shared<std::promise<std::uint32_t>>();
            workIoService->dispatch([this, self = shared_from_this(), serviceId, workIoService, promise, builder, name]
            {
                auto service = builder(*this, *workIoService, serviceId, name);
                // 保存服务
                {
                    auto slot = serviceId % this->services_.size();
                    std::lock_guard<cr::concurrent::MultiMutex<std::mutex>> locker(mutexs_);
                    this->services_[slot].insert(std::make_pair(serviceId, service));
                    this->names[name].insert(serviceId);
                }
                // 启动服务
                service->onStart();
                this->onServiceStart(service);
                promise->set_value(serviceId);
            });
            return promise->get_future();
        }

        void Application::stopServiceNoGuard(std::uint32_t serviceId, std::function<void(std::shared_ptr<Service>)> handler)
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
            auto self = shared_from_this();
            workThreadEntry.first.post([this, self, handler = std::move(handler), service = std::move(service)]
            {
                handler(service);
                service->onStop();
            });
            // 移除服务Id
            workThreadEntry.second.erase(serviceId);
            if (workThreadEntry.second.empty())
            {
                workThreadEntry.first.post([this, self, thread = workThreadEntry.first]
                {
                    backgroundThread_.post([thread = std::move(thread)]() mutable
                    {
                        thread.stop();
                        thread.join();
                    });
                });
                workThreads_.erase(workThreadIter);
            }
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
    }
}