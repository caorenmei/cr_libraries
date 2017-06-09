#include <cr/network/spawn.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/coroutine/asymmetric_coroutine.hpp>

#include <cr/common/assert.h>

namespace cr
{
    namespace network
    {
        typedef boost::coroutines::coroutine<void>::pull_type CoroCaller;
        typedef boost::coroutines::coroutine<void>::push_type CoroCallee;

        class Coroutine::Impl
        {
        public:

            Impl(boost::asio::io_service::strand strand, CoroCaller& caller, std::weak_ptr<CoroCallee> callee)
                : status_(Status::RUNNING),
                strand_(strand),
                caller_(caller),
                callee_(std::move(callee))
            {}

            ~Impl()
            {}

            Impl(const Impl&) = delete;
            Impl& operator=(const Impl&) = delete;

            void resume()
            {
                CR_ASSERT(status_ == Status::SUSPENDED)(static_cast<int>(status_));
                status_ = Status::RUNNING;
                auto callee = callee_.lock();
                CR_ASSERT(callee != nullptr);
                (*callee)();
            }

            void yield()
            {
                CR_ASSERT(status_ == Status::RUNNING)(static_cast<int>(status_));
                status_ = Status::SUSPENDED;
                caller_();
            }

            enum class Status
            {
                RUNNING = 0,
                SUSPENDED = 1,
            };

            boost::asio::io_service::strand& getIoServieStrand()
            {
                return strand_;
            }

        private:

            Status status_;
            boost::asio::io_service::strand strand_;
            CoroCaller& caller_;
            std::weak_ptr<CoroCallee> callee_;
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
            impl->getIoServieStrand().dispatch([impl] {impl->yield(); });
        }

        void Coroutine::resume()
        {
            auto impl = impl_.lock();
            CR_ASSERT(impl != nullptr);
            impl->getIoServieStrand().dispatch([impl] {impl->resume(); });
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
                auto impl = std::make_shared<Coroutine::Impl>(fun->strand, caller, fun->callee);
                Coroutine coro(impl);
                fun->handler(coro);
                impl->getIoServieStrand().post([fun] {fun->callee.reset(); });
            }, attrs);
            fun->callee = callee;
            strand.dispatch([fun]
            {
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