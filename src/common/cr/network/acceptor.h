#ifndef CR_COMMON_NETOWORK_ACCEPTOR_H_
#define CR_COMMON_NETOWORK_ACCEPTOR_H_

#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace cr
{
    namespace network
    {
        /** tcp 连接器 */
        class Acceptor : public std::enable_shared_from_this<Acceptor>
        {
        public:

            /** 连接处理器 */
            using AcceptHandler = std::function<void(boost::asio::ip::tcp::socket)>;

            /**
             * 构造函数
             * @param ioService io service
             * @param endpoint 监听地址
             */
            Acceptor(boost::asio::io_service& ioService, const boost::asio::ip::tcp::endpoint& endpoint);

            /** 析构函数 */
            ~Acceptor();

            Acceptor(const Acceptor&) = delete;
            Acceptor& operator=(const Acceptor&) = delete;

            /** 开始监听端口 */
            void start();

            /** 停止接收端口 */
            void stop();

            /**
             * 设置监听处理器
             * @param handler 监听处理器 
             */
            void setAcceptHandler(AcceptHandler handler);

        private:

            // 连接回调
            void onAcceptHandler(const boost::system::error_code& error);

            // 接收器
            boost::asio::ip::tcp::acceptor acceptor_;
            // 地址
            boost::asio::ip::tcp::endpoint endpoint_;
            // 套接字
            boost::asio::ip::tcp::socket socket_;
            // 运行态
            bool running_;
            // 处理器
            AcceptHandler handler_;
        };
    }
}

#endif