#ifndef CR_COMMON_NETWORK_CONNECTOR_H_
#define CR_COMMON_NETWORK_CONNECTOR_H_

#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace cr
{
    namespace network
    {
        /** tcp 连接器 */
        class Connector : public std::enable_shared_from_this<Connector>
        {
        public:

            /**
             * 构造函数
             * @param ioService io service
             * @param endpoint 对端地址
             */
            Connector(boost::asio::io_service& ioService, const boost::asio::ip::tcp::endpoint& endpoint);

            /** 析构函数*/
            ~Connector();

            Connector(const Connector&) = delete;
            Connector& operator=(const Connector&) = delete;

            /**
             * 开始连接
             * @param cb 连接成功回调
             */
            void start(std::function<void(boost::asio::ip::tcp::socket)> cb);

            /**
             * 延迟连接
             * @param cb 连接成功回调
             */
            void delayStart(std::function<void(boost::asio::ip::tcp::socket)> cb);

            /**
             * 停止连接
             */
            void stop();

        private:

            // 开始连接
            void connect();

            // 连接回调
            void onConnectHandler(const boost::system::error_code& error);

            // 重试
            void retry();

            // io service
            boost::asio::io_service& ioService_;
            // 远程地址
            boost::asio::ip::tcp::endpoint endpoint_;
            // socket
            boost::asio::ip::tcp::socket socket_;
            // timer
            boost::asio::steady_timer timer_;
            // 运行状态
            bool running_;
            // 回调
            std::function<void(boost::asio::ip::tcp::socket)> cb_;
        };
    }
}

#endif
