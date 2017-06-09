#ifndef CR_NETWORK_SPAWN_HPP_
#define CR_NETWORK_SPAWN_HPP_

#include <functional>
#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/coroutine2/fixedsize_stack.hpp>
#include <boost/coroutine2/pooled_fixedsize_stack.hpp>
#include <boost/coroutine2/protected_fixedsize_stack.hpp>

namespace cr
{
    namespace network
    {
        /** 协程. */
        class Coroutine
        {
        public:

            /** An implementation. */
            class Impl;

            /**
            * Constructor.
            * @param impl 实际实现.
            */
            explicit Coroutine(std::weak_ptr<Impl> impl);

            /** Destructor. */
            ~Coroutine();

            /** 切换协程. */
            void yield();

            /** 唤醒. */
            void resume();

        private:

            // 调用者
            std::weak_ptr<Impl> impl_;
        };

        /**
         * 发起协程
         * @param strand 调度器.
         * @param handler 协程处理器.
         */
        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler);

        /**
         * 发起协程
         * @param strand 调度器.
         * @param handler 协程处理器.
         * @param alloc 堆栈构造器.
         */
        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler, 
            boost::coroutines2::fixedsize_stack alloc);

        /**
         * 发起协程
         * @param strand 调度器.
         * @param handler 协程处理器.
         * @param alloc 堆栈构造器.
         */
        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler, 
            boost::coroutines2::pooled_fixedsize_stack alloc);

        /**
         * 发起协程
         * @param strand 调度器.
         * @param handler 协程处理器.
         * @param alloc 堆栈构造器.
         */
        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler, 
            boost::coroutines2::protected_fixedsize_stack alloc);

        /**
         * 发起协程
         * @param ioService 调度器.
         * @param handler 协程处理器.
         * @param attr 协程参数.
         */
        inline void spawn(boost::asio::io_service& ioService, std::function<void(Coroutine)> handler)
        {
            spawn(boost::asio::io_service::strand(ioService), std::move(handler));
        }

        /**
         * 发起协程
         * @param ioService 调度器.
         * @param handler 协程处理器.
         * @param alloc 栈构造器
         */
        template <typename StackAllocator>
        inline void spawn(boost::asio::io_service& ioService, std::function<void(Coroutine)> handler, 
            StackAllocator alloc)
        {
            spawn(boost::asio::io_service::strand(ioService), std::move(handler), alloc);
        }

    }
}

#endif