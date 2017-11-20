#include "connection.h"

namespace cr
{
    namespace network
    {

        Connection::Connection(boost::asio::ip::tcp::socket socket)
            : socket_(std::move(socket)),
            state_(NORMAL)
        {
            boost::system::error_code ignoreCode;
            localEndpoint_ = socket_.local_endpoint(ignoreCode);
            remoteEndpoint_ = socket_.remote_endpoint(ignoreCode);
        }

        Connection::~Connection()
        {}

        const boost::asio::ip::tcp::endpoint& Connection::getLocalEndpoint() const
        {
            return localEndpoint_;
        }

        const boost::asio::ip::tcp::endpoint& Connection::getRemoteEndpoint() const
        {
            return remoteEndpoint_;
        }

        void Connection::setMessageHandler(MessageHandler handler)
        {
            messageHandler_ = std::move(handler);
        }

        void Connection::setCloseHandler(CloseHandler handler)
        {
            closeHandler_ = std::move(handler);
        }

        bool Connection::connected() const
        {
            return state_ == CONNECTED;
        }

        void Connection::send(const void* data, std::size_t n)
        {
            if (state_ == CONNECTED)
            {
                auto buffer = secondOutputBuffer_.prepare(n);
                boost::asio::buffer_copy(buffer, boost::asio::buffer(data, n));
                if (firstOutputBuffer_.size() == 0)
                {
                    firstOutputBuffer_.swap(secondOutputBuffer_);
                    startWrite();
                }
            }
        }

        void Connection::start()
        {
            if (state_ == NORMAL)
            {
                state_ = CONNECTED;
                startRead();
            }
        }

        void Connection::close()
        {
            if (state_ == CONNECTED)
            {
                state_ = DISCONNECTING;
                socket_.close();
            }
        }

        void Connection::startRead()
        {
            socket_.async_receive(inputBuffer_.prepare(4 * 1024), 
                [this, self = shared_from_this()](const boost::system::error_code& error, std::size_t length)
            {
                if (!error)
                {
                    onReadHandler(length);
                }
                else
                {
                    onErrorHandler(error);
                }
            });
        }

        void Connection::onReadHandler(std::size_t length)
        {
            if (state_ == CONNECTED)
            {
                inputBuffer_.commit(length);
                messageHandler_(shared_from_this(), std::ref(inputBuffer_));
            }
            if (state_ == CONNECTED)
            {
                startRead();
            }
        }

        void Connection::startWrite()
        {
            socket_.async_write_some(firstOutputBuffer_.data(), [this, self = shared_from_this()](const boost::system::error_code& error, std::size_t length)
            {
                if (!error)
                {
                    onWriteHandler(length);
                }
                else
                {
                    onErrorHandler(error);
                }
            });
        }

        void Connection::onWriteHandler(std::size_t length)
        {
            firstOutputBuffer_.consume(length);
            if (firstOutputBuffer_.size() == 0)
            {
                firstOutputBuffer_.swap(secondOutputBuffer_);
            }
            if (firstOutputBuffer_.size() != 0)
            {
                startWrite();
            }
        }

        void Connection::onErrorHandler(const boost::system::error_code& error)
        {
            if (state_ != DISCONNECTED)
            {
                state_ = DISCONNECTED;
                if (closeHandler_)
                {
                    closeHandler_(shared_from_this());
                }
                messageHandler_ = nullptr;
                closeHandler_ = nullptr;
            }
        }
    }
}