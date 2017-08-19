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
                : ioService_(ioService)
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
            void push(T element)
            {
                std::lock_guard<Mutex> locker(mutex_);
                if (!handlers_.empty())
                {
                    auto handler = std::move(handlers_.front());
                    handlers_.pop_front();
                    handler.first->push_back(std::move(element));
                    ioService_.post(std::bind(std::move(handler.second), boost::system::error_code()));
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
                else
                {
                    elements_.emplace_back(std::move(element));
                }
            }

            // 添加元素
            template <typename...  Args>
            void emplace(Args&&... args)
            {
                std::lock_guard<Mutex> locker(mutex_);
                if (!handlers_.empty())
                {
                    auto handler = std::move(handlers_.front());
                    handlers_.pop_front();
                    handler.first->emplace_back(std::forward<Args>(args)...);
                    ioService_.post(std::bind(std::move(handler.second), boost::system::error_code()));
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
                else
                {
                    elements_.emplace_back(std::forward<Args>(args)...);
                }
            }

            // 一次添加多个元素
            template <typename ForwardIterator>
            void push(ForwardIterator first, ForwardIterator last)
            {
                std::lock_guard<Mutex> locker(mutex_);
                if (!handlers_.empty())
                {
                    auto handler = std::move(handlers_.front());
                    handlers_.pop_front();
                    handler.first->insert(handler.first->end(), first, last);
                    ioService_.post(std::bind(std::move(handler.second), boost::system::error_code()));
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
                else
                {
                    elements_.insert(elements_.end(), first, last);
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
                using Signature = void(const boost::system::error_code&);
                boost::asio::detail::async_result_init<PopHandler, Signature> init(std::forward<PopHandler>(handler));
                {
                    std::lock_guard<Mutex> locker(mutex_);
                    if (interrupt_)
                    {
                        namespace asio_error = boost::asio::error;
                        auto errorCode = asio_error::make_error_code(asio_error::interrupted);
                        ioService_.post(std::bind(std::move(init.handler), errorCode));
                    }
                    else if (!elements_.empty())
                    {
                        std::move(elements_.begin(), elements_.end(), std::back_inserter(elements));
                        ioService_.post(std::bind(std::move(init.handler), boost::system::error_code()));
                        elements_.clear();
                    }
                    else
                    {
                        if (work_ == nullptr)
                        {
                            work_ = std::make_unique<boost::asio::io_service::work>(ioService_);
                        }
                        handlers_.emplace_back(&elements, std::move(init.handler));
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
                    namespace asio_error = boost::asio::error;
                    auto errorCode = asio_error::make_error_code(asio_error::operation_aborted);
                    ioService_.post(std::bind(std::move(handler.second), errorCode));
                }
                work_.reset();
                handlers_.clear();
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
             * 中断,设置中断标志位
             * @return 中断的处理器个数
             */
            std::size_t interrupt()
            {
                std::lock_guard<Mutex> locker(mutex_);
                std::size_t handlerCount = handlers_.size();
                for (auto& handler : handlers_)
                {
                    namespace asio_error = boost::asio::error;
                    auto errorCode = asio_error::make_error_code(asio_error::interrupted);
                    ioService_.post(std::bind(std::move(handler.second), errorCode));
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
            using HandlerType = std::function<void(const boost::system::error_code&)>;
            std::deque<std::pair<std::vector<T>*, HandlerType>> handlers_;
            // 中断
            bool interrupt_;
            // 锁
            mutable Mutex mutex_;
        };
    }
}

#endif
