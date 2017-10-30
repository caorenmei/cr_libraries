#ifndef CR_CONCURRENT_THREAD_H_
#define CR_CONCURRENT_THREAD_H_

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio/io_service.hpp>

namespace cr
{
    namespace concurrent
    {
        /**
         * 线程
         */
        class Thread
        {
        public:

            /**
             * 构造函数
             */
            explicit Thread(std::size_t threadNum = 1);

            /**
             * 构造函数, 不会创建线程
             * @param ioService 被包装的io service
             */
            explicit Thread(std::shared_ptr<boost::asio::io_service> ioService);

            /**
             * 析构函数
             */
            ~Thread();

            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;

            /**
             * 获取线程关联的io service
             */
            const std::shared_ptr<boost::asio::io_service>& getIoService() const;

            /**
             * 获取线程数
             * @param 线程数
             */
            std::size_t getThreadNum() const;

            /**
             * 提交一个任务到线程
             * @param handler 任务
             */
            void post(std::function<void()> handler);

            /**
             * 停止io service
             * 忽略构造函数传入的io service
             */
            void stop();

            /**
             * 分离线程
             */
            void detach();

            /**
             * 合并线程
             */
            void join();

        private:

            // io service
            std::shared_ptr<boost::asio::io_service> ioService_;
            // 实际线程
            std::vector<std::unique_ptr<std::thread>> threads_;
        };
    }
}

#endif
