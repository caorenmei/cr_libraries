#ifndef CR_NETWORK_CONNECTION_H_
#define CR_NETWORK_CONNECTION_H_

#include <functional>
#include <memory>

#include <boost/asio.hpp>

#include "byte_buffer.h"

namespace cr
{
    namespace network
    {
        /** socket连接类 */
        class Connection : public std::enable_shared_from_this<Connection>
        {
        public:

            /** 消息处理回调 */
            using MessageHandler = std::function<void(const std::shared_ptr<Connection>&, ByteBuffer&)>;

            /** 连接回调 */
            using CloseHandler = std::function<void(const std::shared_ptr<Connection>&)>;

            /**
             * 构造函数
             * @param socket socket
             */
            explicit Connection(boost::asio::ip::tcp::socket socket);

            /** 析构函数 */
            ~Connection();

            Connection(const Connection&) = delete;
            Connection& operator=(const Connection&) = delete;

            /**
             * 获取本端地址
             * @return 本端地址
             */
            const boost::asio::ip::tcp::endpoint& getLocalEndpoint() const;

            /**
             * 获取远端地址
             * @return 远端地址
             */
            const boost::asio::ip::tcp::endpoint& getRemoteEndpoint() const;

            /**
             * 设置消息处理器 
             * @param handler 消息处理器
             */
            void setMessageHandler(MessageHandler handler);

            /**
             * 设置连接断开回调
             * @param handler 连接断开回调
             */
            void setCloseHandler(CloseHandler handler);

            /**
             * 客户端是否连接上
             * @return true是，false其他
             */
            bool connected() const;

            /**
             * 发送消息
             * @param data 缓冲区
             * @param n 缓冲区长度
             */
            void send(const void* data, std::size_t n);

            /**
             * 开始读取数据
             */
            void start();

            /**
             * 关闭socket
             */
            void close();

        private:

            // 开始读
            void startRead();

            // 输入回调
            void onReadHandler(std::size_t length);

            //开始写
            void startWrite();

            // 输出回调
            void onWriteHandler(std::size_t length);

            // 错误处理器
            void onErrorHandler(const boost::system::error_code& error);

            // socket 
            boost::asio::ip::tcp::socket socket_;
            // 本端地址
            boost::asio::ip::tcp::endpoint localEndpoint_;
            // 远端地址
            boost::asio::ip::tcp::endpoint remoteEndpoint_;
            // 状态
            enum State { NORMAL, CONNECTED, DISCONNECTING, DISCONNECTED };
            State state_;
            // 输入缓冲区
            cr::network::ByteBuffer inputBuffer_;
            // 输出缓冲区
            cr::network::ByteBuffer firstOutputBuffer_;
            cr::network::ByteBuffer secondOutputBuffer_;
            // 消息处理器
            MessageHandler messageHandler_;
            // 关闭处理器
            CloseHandler closeHandler_;
        };
    }
}

#endif
