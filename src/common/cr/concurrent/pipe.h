#ifndef COMMON_CONCURRENT_PIPE_H_
#define COMMON_CONCURRENT_PIPE_H_

#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <boost/asio/io_service.hpp>
#include <boost/thread/null_mutex.hpp>

namespace cr
{
    namespace concurrent
    {
       
        /** 管道类，用作线程间通信 */
        template <typename T, typename Mutex = boost::null_mutex>
        class Pipe
        {
        public:

            /**
             *  构造函数
             * @param ioService io service
             */
            explicit Pipe(boost::asio::io_service& ioService)
                : ioService_(ioService),
                interrupt_(false)
            {}

            /** 析构函数 */
            ~Pipe()
            {
                cancel();
            }

            Pipe(const Pipe&) = delete;
            Pipe& operator=(const Pipe&) = delete;

            /**
             * 添加一个元素
             * @param element 元素
             */
            template <typename U>
            void push(U&& element)
            {
                std::lock_guard<Mutex> locker(mutex_);
                elements_.emplace_back(std::forward<U>(element));
                if (!handlers_.empty())
                {
                    ioService_.post([handler = std::move(handlers_.front()), elements = std::move(elements_)]() mutable
                    {
                        std::move(elements.begin(), elements.end(), std::back_inserter(*handler.first));
                        handler.second(boost::system::error_code(), elements.size());
                    });
                    handlers_.pop_front();
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
            }

            /**
             * 添加元素
             * @param args... 构造元素参数
             */
            template <typename...  Args>
            void emplace(Args&&... args)
            {
                std::lock_guard<Mutex> locker(mutex_);
                elements_.emplace_back(std::forward<Args>(args)...);
                if (!handlers_.empty())
                {
                    ioService_.post([handler = std::move(handlers_.front()), elements = std::move(elements_)]() mutable
                    {
                        std::move(elements.begin(), elements.end(), std::back_inserter(*handler.first));
                        handler.second(boost::system::error_code(), elements.size());
                    });
                    handlers_.pop_front();
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
            }

            /**
             * 一次添加多个元素
             * @param first 起始迭代器
             * @param last 终止迭代器
             */
            template <typename ForwardIterator>
            void push(ForwardIterator first, ForwardIterator last)
            {
                std::lock_guard<Mutex> locker(mutex_);
                std::copy(first, last, std::back_inserter(elements_));
                if (!handlers_.empty())
                {
                    ioService_.post([handler = std::move(handlers_.front()), elements = std::move(elements_)]() mutable
                    {
                        std::move(elements.begin(), elements.end(), std::back_inserter(*handler.first));
                        handler.second(boost::system::error_code(), elements.size());
                    });
                    handlers_.pop_front();
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
            }

            /**
             * 取出元素
             * @param elements 输出容器
             * @param handler 处理器
             * @return void-or-deduced
             */
            template <typename PopHandler>
            auto pop(std::vector<T>& elements, PopHandler&& handler)
            {
                using Signature = void(const boost::system::error_code&, std::size_t);
                boost::asio::detail::async_result_init<PopHandler, Signature> init(std::forward<PopHandler>(handler));
                {
                    std::lock_guard<Mutex> locker(mutex_);
                    handlers_.emplace_back(&elements, std::move(init.handler));
                    if (interrupt_)
                    {
                        ioService_.post([handler = std::move(handlers_.front())]
                        {
                            handler.second(boost::asio::error::make_error_code(boost::asio::error::operation_aborted), 0);
                        });
                        handlers_.pop_front();
                        interrupt_ = false;
                    }
                    else if (!elements_.empty())
                    {
                        ioService_.post([handler = std::move(handlers_.front()), elements = std::move(elements_)]() mutable
                        {
                            std::move(elements.begin(), elements.end(), std::back_inserter(*handler.first));
                            handler.second(boost::system::error_code(), elements.size());
                        });
                        handlers_.pop_front();
                    }
                    else
                    {
                        if (work_ == nullptr)
                        {
                            work_ = std::make_unique<boost::asio::io_service::work>(ioService_);
                        }
                    }
                }
                return init.result.get();
            }

            /**
             * 取消等待
             * @return 取消的处理器个数
             */
            std::size_t cancel()
            {
                std::lock_guard<Mutex> locker(mutex_);
                std::size_t handlerCount = handlers_.size();
                for (auto& handler : handlers_)
                {
                    ioService_.post([handler = std::move(handler.second)]
                    {
                        handler(boost::asio::error::make_error_code(boost::asio::error::operation_aborted), 0);
                    });
                }
                work_.reset();
                handlers_.clear();
                return handlerCount;
            }

            /**
             * 中断,设置中断标志位
             * @return 中断的处理器个数
             */
            std::size_t interrupt()
            {
                std::lock_guard<Mutex> locker(mutex_);
                std::size_t handlerCount = handlers_.size();
                for (auto& handler : handlers_)
                {
                    ioService_.post([handler = std::move(handler.second)]
                    {
                        handler(boost::asio::error::make_error_code(boost::asio::error::operation_aborted), 0);
                    });
                }
                work_.reset();
                handlers_.clear();
                if (handlerCount == 0)
                {
                    interrupt_ = true;
                }
                return handlerCount;
            }

            /**
             * 清空元素
             */
            void clear()
            {
                std::lock_guard<Mutex> locker(mutex_);
                elements_.clear();
            }

            /**
             * 元素个数
             */
            std::size_t size() const
            {
                std::lock_guard<Mutex> locker(mutex_);
                return elements_.size();
            }

            /**
             * 队列是否为空
             */
            bool empty() const
            {
                std::lock_guard<Mutex> locker(mutex_);
                return elements_.empty();
            }

            /**
             * io service
             */
            boost::asio::io_service& getIoService()
            {
                return ioService_;
            }

        private:

            // 调度器
            boost::asio::io_service& ioService_;
            std::unique_ptr<boost::asio::io_service::work> work_;
            // 消息
            std::deque<T> elements_;
            // 处理器
            using HandlerType = std::function<void(const boost::system::error_code&, std::size_t)>;
            std::deque<std::pair<std::vector<T>*, HandlerType>> handlers_;
            // 中断
            bool interrupt_;
            // 锁
            mutable Mutex mutex_;
        };
    }
}

#endif
