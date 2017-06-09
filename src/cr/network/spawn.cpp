#include <cr/network/spawn.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/coroutine2/coroutine.hpp>

#include <cr/common/assert.h>

namespace cr
{
    namespace network
    {
        typedef boost::coroutines2::coroutine<void>::pull_type CoroCaller;
        typedef boost::coroutines2::coroutine<void>::push_type CoroCallee;

        class Coroutine::Impl
        {
        public:

            Impl(boost::asio::io_service::strand strand, CoroCaller& caller, std::shared_ptr<CoroCallee> callee)
                : status(Status::RUNNING),
                strand(strand),
                caller(caller),
                callee(callee)
            {}

            ~Impl()
            {}

            Impl(const Impl&) = delete;
            Impl& operator=(const Impl&) = delete;

            void resume()
            {
                CR_ASSERT(status != Status::SUSPENDED);
                (*callee)();
            }

            void yield()
            {
                CR_ASSERT(status != Status::RUNNING);
                caller();
            }

            // 状态
            enum class Status
            {
                // 运行态
                RUNNING = 0,
                // 休眠态
                SUSPENDED = 1,
            };

            Status status;
            boost::asio::io_service::strand strand;
            CoroCaller& caller;
            std::shared_ptr<CoroCallee> callee;
        };

        Coroutine::Coroutine(std::weak_ptr<Impl> impl)
            : impl_(std::move(impl))
        {}

        Coroutine::~Coroutine()
        {}

        void Coroutine::yield()
        {
            auto impl = impl_.lock();
            CR_ASSERT(impl != nullptr);
            impl->strand.dispatch([impl] {impl->yield(); });
        }

        void Coroutine::resume()
        {
            auto impl = impl_.lock();
            CR_ASSERT(impl != nullptr);
            impl->strand.dispatch([impl] {impl->resume(); });
        }

        struct CoroutineFun
        {
        public:

            CoroutineFun(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler)
                : strand(strand),
                handler(std::move(handler))
            {}

            void start()
            {
                (*callee)();
            }

            std::shared_ptr<CoroCallee> callee;
            boost::asio::io_service::strand strand;
            std::function<void(Coroutine)> handler;
        };

        namespace detail
        {
            template <typename StackAllocator>
            void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler, StackAllocator alloc)
            {
                CR_ASSERT(handler != nullptr);
                auto fun = std::make_shared<CoroutineFun>(strand, handler);
                auto callee = std::make_shared<CoroCallee>(alloc, [fun](CoroCaller& caller)
                {
                    auto impl = std::make_shared<Coroutine::Impl>(fun->strand, caller, fun->callee);
                    Coroutine coro(impl);
                    fun->handler(coro);
                });
                fun->callee = callee;
                strand.dispatch([fun]
                {
                    fun->start();
                });
            }
        }

        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler)
        {
            detail::spawn(strand, std::move(handler), boost::coroutines2::default_stack());
        }

        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler,
            boost::coroutines2::fixedsize_stack alloc)
        {
            detail::spawn(strand, std::move(handler), alloc);
        }

        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler,
            boost::coroutines2::pooled_fixedsize_stack alloc)
        {
            detail::spawn(strand, std::move(handler), alloc);
        }

        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler, 
            boost::coroutines2::protected_fixedsize_stack alloc)
        {
            detail::spawn(strand, std::move(handler), alloc);
        }
    }
}