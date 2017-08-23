#include "thread.h"

namespace cr
{
    namespace concurrent
    {

        Thread::Thread()
        {
            ioService_ = std::make_shared<boost::asio::io_service>();
            thread_ = std::make_shared<std::thread>([this]
            {
                boost::asio::io_service::work work(*ioService_);
                ioService_->run();
            });
        }

        Thread::Thread(std::shared_ptr<boost::asio::io_service> ioService)
            : ioService_(std::move(ioService))
        {}

        Thread::~Thread()
        {}

        const std::shared_ptr<boost::asio::io_service>& Thread::getIoService() const
        {
            return ioService_;
        }

        void Thread::post(std::function<void()> handler)
        {
            ioService_->post(std::move(handler));
        }

        void Thread::stop()
        {
            if (thread_)
            {
                ioService_->stop();
            }
        }

        void Thread::join()
        {
            if (thread_)
            {
                thread_->join();
            }
        }
    }
}