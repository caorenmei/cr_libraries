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

#include <cr/common/assert.h>

namespace cr
{
    namespace concurrent
    {
        template <typename T, typename Mutex = boost::null_mutex>
        class Pipe
        {
        public:

            explicit Pipe(boost::asio::io_service& ioService)
                : ioService_(ioService)
            {}

            ~Pipe()
            {
                cancel();
            }

            Pipe(const Pipe&) = delete;
            Pipe& operator=(const Pipe&) = delete;

            // 添加元素
            template <typename U>
            void push(U&& element)
            {
                std::lock_guard<Mutex> locker(mutex_);
                if (!handlers_.empty())
                {
                    CR_ASSERT(elements_.empty() && work_ != nullptr)(elements_.size());
                    auto handler = std::move(handlers_.front());
                    handlers_.pop_front();
                    handler.first->push_back(std::forward<U>(element));
                    ioService_.post(std::bind(std::move(handler.second), boost::system::error_code()));
                    if (handlers_.empty())
                    {
                        work_.reset();
                    }
                }
                else
                {
                    CR_ASSERT(work_ == nullptr);
                    elements_.emplace_back(std::forward<U>(element));
                }
            }

            // 一次添加多个元素
            template <typename ForwardIterator>
            void push(ForwardIterator first, ForwardIterator last)
            {
                std::lock_guard<Mutex> locker(mutex_);
                if (!handlers_.empty())
                {
                    CR_ASSERT(elements_.empty() && work_ != nullptr)(elements_.size());
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
                    CR_ASSERT(work_ == nullptr);
                    elements_.insert(elements_.end(), first, last);
                }
            }

            // 取出元素
            template <typename PopHandler>
            auto pop(std::vector<T>& elements, PopHandler&& handler)
            {
                using boost::asio::detail::async_result_init;
                using Signature = void(const boost::system::error_code&);
                async_result_init<PopHandler, Signature> init(std::forward<PopHandler>(handler));
                {
                    std::lock_guard<Mutex> locker(mutex_);
                    if (!elements_.empty())
                    {
                        CR_ASSERT(handlers_.empty() && work_ == nullptr)(handlers_.size());
                        std::move(elements_.begin(), elements_.end(), std::back_inserter(elements));
                        ioService_.post(std::bind(std::move(init.handler), boost::system::error_code()));
                        elements_.clear();
                    }
                    else
                    {
                        if (work_ == nullptr)
                        {
                            CR_ASSERT(handlers_.empty())(handlers_.size());
                            work_ = std::make_unique<boost::asio::io_service::work>(ioService_);
                        }
                        handlers_.emplace_back(&elements, std::move(init.handler));
                    }
                }
                return init.result.get();
            }

            // 取消等待
            std::size_t cancel()
            {
                std::size_t handlerCount = handlers_.size();
                namespace asio_error = boost::asio::error;
                auto errorCode = asio_error::make_error_code(asio_error::operation_aborted);
                std::lock_guard<Mutex> locker(mutex_);
                for (auto& handler : handlers_)
                {
                    ioService_.post(std::bind(std::move(handler.second), errorCode));
                }
                work_.reset();
                handlers_.clear();
                return handlerCount;
            }

            // 清空元素
            void clear()
            {
                std::lock_guard<Mutex> locker(mutex_);
                elements_.clear();
            }

            // 元素个数
            std::size_t size() const
            {
                std::lock_guard<Mutex> locker(mutex_);
                return elements_.size();
            }

            // 队列是否为空
            bool empty() const
            {
                std::lock_guard<Mutex> locker(mutex_);
                return elements_.empty();
            }

            // io service
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
            // 锁
            mutable Mutex mutex_;
        };
    }
}

#endif
