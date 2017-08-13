#include "application.h"

#include <algorithm>
#include <functional>
#include <thread>

#include <boost/core/null_deleter.hpp>
#include <boost/functional/hash.hpp>

#include <cr/common/assert.h>

#include "local_cluster.h"
#include "service.h"

namespace cr
{
    namespace app
    {
        // 线程
        struct ThreadHolder
        {
            std::shared_ptr<std::thread> thread;
            std::shared_ptr<boost::asio::io_service> ioService;

            void run()
            {
                boost::asio::io_service::work work(*ioService);
                ioService->run();
                thread.reset();
            }
        };

        class LockGuards
        {
        public:

            explicit LockGuards(std::vector<std::unique_ptr<std::mutex>>& mutexs)
                : mutexs_(mutexs)
            {
                std::for_each(mutexs_.begin(), mutexs_.end(), [](std::unique_ptr<std::mutex>& mutex)
                {
                    mutex->lock();
                });
            }

            ~LockGuards()
            {
                std::for_each(mutexs_.rbegin(), mutexs_.rend(), [](std::unique_ptr<std::mutex>& mutex)
                {
                    mutex->unlock();
                });
            }

            LockGuards(const LockGuards&) = delete;
            LockGuards& operator=(const LockGuards&) = delete;

        private:
            std::vector<std::unique_ptr<std::mutex>>& mutexs_;
        };

        // 分段锁的个数
        constexpr std::size_t gMutexSlotNum = 16;

        // 槽索引
        template <typename Key>
        std::size_t calSlotIndex(const Key& key)
        {
            return boost::hash<Key>()(key) % gMutexSlotNum;
        }

        Application::Application(boost::asio::io_service& ioService, std::string name)
            : ioService_(&ioService, boost::null_deleter()),
            logger_(name),
            cluster_(std::make_shared<LocalCluster>(*ioService_)),
            nextServiceId_(0),
            services_(gMutexSlotNum)
        {
            std::string mainThreadGroup = "Main";
            threads_.insert(std::make_pair(mainThreadGroup, std::make_pair(ioService_, 1)));
            // 初始化锁
            for (auto&& services : services_)
            {
                mutexs_.push_back(std::make_unique<std::mutex>());
            }
            // 集群的消息分发器
            cluster_->setMessageDispatcher(std::bind(&Application::dispatchMessage, this, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, 
                std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
        }

        Application::~Application()
        {}

        boost::asio::io_service& Application::getIoService()
        {
            return *ioService_;
        }

        void Application::onStart()
        {
            CR_ASSERT(work_ == nullptr);
            work_ = std::make_unique<boost::asio::io_service::work>(*ioService_);
        }

        void Application::onStop()
        {
            LockGuards locker(mutexs_);
            CR_ASSERT(work_ != nullptr);
            // 停止所有服务
            std::for_each(serviceIds_.rbegin(), serviceIds_.rend(), [this](auto serviceId)
            {
                this->stopServiceNoGuard(serviceId);
            });
            work_.reset();
        }

        void Application::onServiceStart(std::shared_ptr<Service> service)
        {
            {
                auto slotIndex = calSlotIndex(service->getId());
                LockGuards locker(mutexs_);
                auto serviceIter = services_[slotIndex].find(service->getId());
                CR_ASSERT(serviceIter == services_[slotIndex].end())(service->getName())(service->getId());
                // 保存服务
                services_[slotIndex].insert(std::make_pair(service->getId(), service));
                serviceIds_.push_back(service->getId());
                serviceNames_[service->getName()].insert(service->getId());
            }
            service->onStart();
        }

        void Application::onServiceStop(std::shared_ptr<Service> service)
        {
            service->onStop();
            {
                auto slotIndex = calSlotIndex(service->getId());
                LockGuards locker(mutexs_);
                auto serviceIter = services_[slotIndex].find(service->getId());
                CR_ASSERT(serviceIter != services_[slotIndex].end())(service->getName())(service->getId());
                // 退役服务列表
                stopServiceIds_.erase(service->getId());
                // 服务 id
                auto serviceIdIter = std::find(serviceIds_.begin(), serviceIds_.end(), service->getId());
                CR_ASSERT(serviceIdIter != serviceIds_.end())(service->getId());
                serviceIds_.erase(serviceIdIter);
                // 服务名字
                auto serviceNameIter = serviceNames_.find(service->getName());
                CR_ASSERT(serviceNameIter != serviceNames_.end())(service->getName());
                serviceNameIter->second.erase(service->getId());
                if (serviceNameIter->second.empty())
                {
                    serviceNames_.erase(serviceNameIter);
                }
                // 服务
                services_[slotIndex].erase(service->getId());
                // 线程组
                auto threadGroupIter = threadGroups_.find(service->getId());
                CR_ASSERT(threadGroupIter != threadGroups_.end())(service->getId());
                auto threadGroup = threadGroupIter->second;
                threadGroups_.erase(threadGroupIter);
                // 线程
                auto threadIter = threads_.find(threadGroup);
                CR_ASSERT(threadIter != threads_.end())(threadGroup);
                auto thread = threadIter->second.first;
                // 减少引用计数
                if (--threadIter->second.second == 0)
                {
                    thread->stop();
                    threads_.erase(threadIter);
                }
            } 
        }

        std::future<std::uint32_t> Application::startService(const std::string& threadGroup, 
            std::function<std::shared_ptr<Service>(Application&, boost::asio::io_service&, std::uint32_t)> factory)
        {
            auto result = std::make_shared<std::promise<std::uint32_t>>();
            ioService_->post([this, self = shared_from_this(), threadGroup, factory, result]
            {
                std::uint32_t serviceId = 0;
                std::shared_ptr<boost::asio::io_service> thread;
                {
                    LockGuards locker(mutexs_);
                    CR_ASSERT(this->work_ != nullptr);
                    serviceId = nextServiceId_++;
                    // 创建线程
                    threadGroups_.insert(std::make_pair(serviceId, threadGroup));
                    auto threadIter = threads_.find(threadGroup);
                    if (threadIter == threads_.end())
                    {
                        auto ioService = std::make_shared<boost::asio::io_service>();
                        auto threadHolder = std::make_shared<ThreadHolder>();
                        threadHolder->ioService = ioService;
                        threadHolder->thread = std::make_shared<std::thread>(std::bind(&ThreadHolder::run, threadHolder));
                        threadHolder->thread->detach();
                        threadIter = threads_.insert(std::make_pair(threadGroup, std::make_pair(ioService, 0))).first;
                    }
                    ++threadIter->second.second;
                    thread = threadIter->second.first;
                }
                // 在线程创建服务
                thread->dispatch([this, self, thread, serviceId, factory, result]
                {
                    auto service = factory(*this, *thread, serviceId);
                    this->onServiceStart(service);
                    result->set_value(serviceId);
                });
            });
            return result->get_future();
        }

        void Application::stopService(std::uint32_t serviceId)
        {
            LockGuards locker(mutexs_);
            stopServiceNoGuard(serviceId);
        }

        std::shared_ptr<Service> Application::getService(std::uint32_t serviceId) const
        {
            auto slotIndex = calSlotIndex(serviceId);
            std::lock_guard<std::mutex> locker(*mutexs_[slotIndex]);
            auto serviceIter = services_[slotIndex].find(serviceId);
            if (serviceIter != services_[slotIndex].end())
            {
                return serviceIter->second;
            }
            return nullptr;
        }

        std::vector<std::uint32_t> Application::getServiceIds() const
        {
            LockGuards locker(mutexs_);
            return { serviceIds_.begin(), serviceIds_.end() };
        }

        std::vector<std::string> Application::getServiceNames() const
        {
            std::vector<std::string> serviceNames;
            LockGuards locker(mutexs_);
            serviceNames.reserve(serviceNames_.size());
            for (const auto& serviceName : serviceNames_)
            {
                serviceNames.push_back(serviceName.first);
            }
            return serviceNames;
        }

        std::vector<std::uint32_t> Application::findServiceIds(const std::string& name) const
        {
            LockGuards locker(mutexs_);
            auto serviceNameIter = serviceNames_.find(name);
            if (serviceNameIter != serviceNames_.end())
            {
                auto serviceIds = serviceNameIter->second;
                return { serviceIds.begin(), serviceIds.end() };
            }
            return {};
        }

        void Application::setCluster(std::shared_ptr<Cluster> cluster)
        {
            cluster_ = std::move(cluster);
        }

        const std::shared_ptr<Cluster>& Application::getCluster() const
        {
            return cluster_;
        }

        void Application::start()
        {
            ioService_->post([this, self = shared_from_this()]
            {
                this->onStart();
            });
        }

        void Application::stop()
        {
            ioService_->post([this, self = shared_from_this()]
            {
                this->onStop();
            });
        }

        void Application::dispatchMessage(std::uint32_t sourceId, std::uint32_t sourceServiceId,
            std::uint32_t destId, std::uint32_t destServiceId,
            std::uint64_t session, std::shared_ptr<Message> message)
        {
            CR_ASSERT(cluster_ != nullptr && destId == 0)(destId);
            auto slotIndex = calSlotIndex(destServiceId);
            std::lock_guard<std::mutex> locker(*mutexs_[slotIndex]);
            auto serviceIter = services_[slotIndex].find(destServiceId);
            if (serviceIter != services_[slotIndex].end())
            {
                serviceIter->second->onServiceMessage(sourceId, sourceServiceId, session, std::move(message));
            }
            else
            {
                CRLOG_WARN(logger_, "dispatchMessage") << "Can't Find Service:"
                    << " SourceId=" << sourceId  
                    << " SourceServiceId=" << sourceServiceId
                    << " DestServiceId=" << destServiceId;
            }
        }

        void Application::stopServiceNoGuard(std::uint32_t serviceId)
        {
            // 服务
            auto slotIndex = calSlotIndex(serviceId);
            auto serviceIter = services_[slotIndex].find(serviceId);
            if (serviceIter == services_[slotIndex].end())
            {
                CRLOG_WARN(logger_, "stopServiec") << "Can't Find Service With: Id=" << serviceId;
                return;
            }
            auto service = serviceIter->second;
            if (!stopServiceIds_.insert(serviceId).second)
            {
                CRLOG_WARN(logger_, "stopServiec") << "Repeated Call stopService() With: Id=" << serviceId;
                return;
            }
            // 线程
            auto threadGroupIter = threadGroups_.find(serviceId);
            CR_ASSERT(threadGroupIter != threadGroups_.end())(serviceId);
            auto threadGroup = threadGroupIter->second;
            auto threadIter = threads_.find(threadGroup);
            CR_ASSERT(threadIter != threads_.end())(threadGroup);
            auto thread = threadIter->second.first;
            // 在服务所在线程调用stop
            thread->post([this, self = shared_from_this(), service, thread]
            {
                this->onServiceStop(std::move(service));
            });
        }
    }
}