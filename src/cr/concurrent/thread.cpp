#include "thread.h"

namespace cr
{
    namespace concurrent
    {

        Thread::Thread(std::size_t threadNum/* = 1*/)
        {
            auto ioService = std::make_shared<boost::asio::io_service>();
            auto work = std::make_shared<boost::asio::io_service::work>(*ioService);
            for (std::size_t i = 0; i < threadNum; ++i)
            {
                threads_.push_back(std::make_shared<std::thread>([ioService, work]
                {
                    ioService->run();
                }));
            }
            ioService_ = std::move(ioService);
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
            if (!threads_.empty())
            {
                ioService_->stop();
            }
        }

        void Thread::join()
        {
            for (auto& thread : threads_)
            {
                thread->join();
            }
        }
    }
}