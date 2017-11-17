#include "connector.h"

namespace cr
{
    namespace network
    {
        Connector::Connector(boost::asio::io_service& ioService, const boost::asio::ip::tcp::endpoint& endpoint)
            : ioService_(ioService),
            endpoint_(endpoint),
            socket_(ioService_),
            timer_(ioService),
            running_(false)
        {}

        Connector::~Connector()
        {}

        void Connector::start(std::function<void(boost::asio::ip::tcp::socket)> cb)
        {
            if (!running_)
            {
                connect();
                running_ = true;
            }
        }

        void Connector::delayStart(std::function<void(boost::asio::ip::tcp::socket)> cb)
        {
            if (!running_)
            {
                retry();
                running_ = true;
            }
        }

        void Connector::stop()
        {
            if (running_)
            {
                boost::system::error_code ignored;
                socket_.close(ignored);
                timer_.cancel();
                running_ = false;
            }
        }

        void Connector::connect()
        {
            socket_.open(endpoint_.protocol());
            socket_.async_connect(endpoint_, [this, self = shared_from_this()](const boost::system::error_code& error)
            {
                onConnectHandler(error);
            });
        }

        void Connector::onConnectHandler(const boost::system::error_code& error)
        {
            if (!error)
            {
                cb_(std::move(socket_));
                running_ = false;
            }
            else if (running_)
            {
                boost::system::error_code ignored;
                socket_.close(ignored);
                retry();
            }
        }

        void Connector::retry()
        {
            timer_.expires_from_now(std::chrono::seconds(1));
            timer_.async_wait([this](const boost::system::error_code& error)
            {
                if (!error)
                {
                    connect();
                }
            });
        }
    }
}