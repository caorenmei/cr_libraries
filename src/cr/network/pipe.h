#ifndef CR_NETWORK_PIPE_HPP_
#define CR_NETWORK_PIPE_HPP_

#include <deque>
#include <functional>
#include <memory>
#include <mutex>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/thread/null_mutex.hpp>

#include <cr/common/assert.h>

namespace cr
{
    namespace network
    {

        /** 管道类，支持协程 */  
        template <typename T, typename TMutex = boost::null_mutex>
        class Pipe
        {
        public:

            /** 完成处理器 */
            using HandlerType = std::function<void(const boost::system::error_code&, T)>;
            
            /**
             * Constructor.
             * @param ioService boost::asio::io_service.
             */
            explicit Pipe(boost::asio::io_service& ioService)
                : strand_(ioService),
                shutdown_(false)
            {}

            /** Destructor. */
            ~Pipe()
            {
                cancel();
            }

            Pipe(const Pipe&) = delete;
            Pipe& operator=(const Pipe&) = delete;

            /**
            * 追加元素到队尾.
            * @param element The element.
            */
            template <typename... TArgs>
            bool push(TArgs&&... args)
            {
                std::function<void()> complete;
                std::unique_ptr<boost::asio::io_service::work> work;
                bool pushed = false;
                {
                    std::lock_guard<TMutex> lg(mutex_);
                    if (!shutdown_)
                    {
                        if (!handlers_.empty())
                        {
                            CR_ASSERT(queue_.empty())(handlers_.size());
                            T telement(std::forward<TArgs>(args)...);
                            complete = [handler = std::move(handlers_.front()), telement = std::move(telement)]() mutable
                            {
                                handler(boost::system::error_code(), std::move(telement));
                            };
                            handlers_.pop_front();
                            if (handlers_.empty())
                            {
                                std::swap(work_, work);
                            }
                        }
                        else
                        {
                            queue_.emplace_back(std::forward<TArgs>(args)...);
                        }
                        pushed = true;
                    }
                }
                if (complete != nullptr)
                {
                    strand_.dispatch(std::move(complete));
                }
                return pushed;
            }

            /**
             * 移除队头元素.
             * @param handler 异步处理器.
             * @return void-or-deduced.
             */
            template <typename PopHandler>
            auto pop(PopHandler&& handler)
            {
                boost::asio::detail::async_result_init<PopHandler,
                    void(const boost::system::error_code&, T)> init(
                        std::forward<PopHandler>(handler));
                std::function<void()> complete;
                {
                    std::lock_guard<TMutex> lg(mutex_);
                    if (!shutdown_)
                    {
                        if (!queue_.empty())
                        {
                            CR_ASSERT(handlers_.empty())(queue_.size());
                            T element = std::move(queue_.front());
                            queue_.pop_front();
                            complete = [handler = std::move(init.handler), element = std::move(element)]() mutable
                            {
                                handler(boost::system::error_code(), std::move(element));
                            };
                        }
                        else
                        {
                            handlers_.emplace_back(std::move(init.handler));
                            if (work_ == nullptr)
                            {
                                work_ = std::make_unique<boost::asio::io_service::work>(get_io_service());
                            }
                        }
                    }
                    else
                    {
                        namespace asio_error = boost::asio::error;
                        complete = std::bind(std::move(init.handler), asio_error::make_error_code(asio_error::broken_pipe), T());
                    }
                }
                if (complete)
                {
                    strand_.dispatch(std::move(complete));
                }
                return init.result.get();
            }

            /**
             * 取消等待的任务.
             * @return 取消任务的个数.
             */
            std::size_t cancel()
            {
                std::deque<HandlerType> handlers;
                std::unique_ptr<boost::asio::io_service::work> work;
                {
                    std::lock_guard<TMutex> lg(mutex_);
                    std::swap(handlers_, handlers);
                    std::swap(work_, work);
                }
                for (auto&& handler : handlers)
                {
                    namespace asio_error = boost::asio::error;
                    auto complete = std::bind(std::move(handler), asio_error::make_error_code(asio_error::operation_aborted), T());
                    strand_.dispatch(complete);
                }
                return handlers.size();
            }

            /**
            * 停止等待的任务.
            * @return 取消任务的个数.
            */
            std::size_t shutdown()
            {
                {
                    std::lock_guard<TMutex> lg(mutex_);
                    queue_.clear();
                    shutdown_ = true;
                }
                return cancel();
            }

            /**
             * 清楚元素.
             */
            void clear()
            {
                std::lock_guard<TMutex> lg(mutex_);
                queue_.clear();
            }

            /**
             * 管道是否打开.
             * @return True 打开，否则其它.
             */
            bool isOpen() const
            {
                std::lock_guard<TMutex> lg(mutex_);
                return !shutdown_;
            }

            /**
             * 元素个数.
             * @return 元素个数.
             */
            std::size_t size() const
            {
                std::lock_guard<TMutex> lg(mutex_);
                return queue_.size();
            }

            /**
             * 管道是否为空.
             * @return True 管道为空, false 其它.
             */
            bool empty() const
            {
                std::lock_guard<TMutex> lg(mutex_);
                return queue_.empty();
            }

            /**
             * Gets io service.
             * @return The io service.
             */
            boost::asio::io_service& get_io_service()
            {
                return strand_;
            }

        private:

            // strad
            boost::asio::io_service& strand_;
            // io_service work
            std::unique_ptr<boost::asio::io_service::work> work_;
            // 处理器
            std::deque<HandlerType> handlers_;
            // 队列
            std::deque<T> queue_;
            //shutdown
            bool shutdown_;
            // 锁
           mutable TMutex mutex_;
        };
    }
}

#endif
