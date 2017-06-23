#include <cr/concurrent/spawn.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <cr/common/assert.h>

namespace cr
{
    namespace concurrent
    {
        typedef boost::coroutines::coroutine<void>::pull_type CoroCaller;
        typedef boost::coroutines::coroutine<void>::push_type CoroCallee;

        class Coroutine::Impl : public std::enable_shared_from_this<Impl>
        {
        public:

            Impl(boost::asio::io_service::strand strand, CoroCaller& caller, CoroCallee& callee)
                : status_(Status::RUNNING),
                strand_(strand),
                caller_(caller),
                callee_(callee)
            {}

            ~Impl()
            {}

            Impl(const Impl&) = delete;
            Impl& operator=(const Impl&) = delete;

            void resume()
            {
                strand_.dispatch([this, self = shared_from_this()]
                {
                    CR_ASSERT(status_ == Status::SUSPENDED)(static_cast<int>(status_));
                    status_ = Status::RUNNING;
                    callee_();
                });
            }

            void yield()
            {
                strand_.dispatch([this, self = shared_from_this()]
                {
                    CR_ASSERT(status_ == Status::RUNNING)(static_cast<int>(status_));
                    status_ = Status::SUSPENDED;
                    caller_();
                });
            }

            void dead()
            {
                CR_ASSERT(status_ == Status::RUNNING)(static_cast<int>(status_));
                status_ = Status::DEAD;
            }

        private:

            enum class Status
            {
                RUNNING = 0,
                SUSPENDED = 1,
                DEAD = 2,
            };

            Status status_;
            boost::asio::io_service::strand strand_;
            CoroCaller& caller_;
            CoroCallee& callee_;
        };

        Coroutine::Coroutine(std::shared_ptr<Impl> impl)
            : impl_(std::move(impl))
        {}

        Coroutine::~Coroutine()
        {}

        void Coroutine::yield()
        {
            impl_->yield();
        }

        void Coroutine::resume()
        {
            impl_->resume();
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

        void spawn(boost::asio::io_service::strand strand, std::function<void(Coroutine)> handler, 
            boost::coroutines::attributes attrs/* = boost::coroutines::attributes()*/)
        {
            CR_ASSERT(handler != nullptr);
            auto fun = std::make_shared<CoroutineFun>(strand, handler);
            auto callee = std::make_shared<CoroCallee>([fun](CoroCaller& caller)
            {
                auto impl = std::make_shared<Coroutine::Impl>(fun->strand, caller, *(fun->callee));
                Coroutine coro(impl);
                fun->handler(coro);
                impl->dead();
                fun->strand.post([fun] {fun->callee.reset(); });
            }, attrs);
            strand.dispatch([fun, callee]
            {
                fun->callee = callee;
                fun->start();
            });
        }

        void spawn(boost::asio::io_service& ioService, std::function<void(Coroutine)> handler,
            boost::coroutines::attributes attrs/* = boost::coroutines::attributes()*/)
        {
            spawn(boost::asio::io_service::strand(ioService), std::move(handler), attrs);
        }
    }
}