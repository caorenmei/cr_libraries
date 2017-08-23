#ifndef CR_CONCURRENT_THREAD_H_
#define CR_CONCURRENT_THREAD_H_

#include <functional>
#include <memory>
#include <thread>

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
            Thread();

            /**
             * 构造函数, 不会创建线程
             * @param ioService 被包装的io service
             */
            explicit Thread(std::shared_ptr<boost::asio::io_service> ioService);

            /**
             * 析构函数
             */
            ~Thread();

            /**
             * 获取线程关联的io service
             */
            const std::shared_ptr<boost::asio::io_service>& getIoService() const;

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
             * 合并线程
             */
            void join();

        private:

            // io service
            std::shared_ptr<boost::asio::io_service> ioService_;
            // 实际线程
            std::shared_ptr<std::thread> thread_;
        };
    }
}

#endif
