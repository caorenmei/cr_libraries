#include "acceptor.h"

namespace cr
{
    namespace network
    {

        Acceptor::Acceptor(boost::asio::io_service& ioService, const boost::asio::ip::tcp::endpoint& endpoint)
            : acceptor_(ioService),
            endpoint_(endpoint),
            socket_(ioService),
            running_(false)
        {
            acceptor_.open(endpoint_.protocol());
            acceptor_.non_blocking(true);
        }

        Acceptor::~Acceptor()
        {}

        void Acceptor::start()
        {
            if (!running_)
            {
                running_ = true;
                acceptor_.listen();
                acceptor_.async_accept(socket_, [this, self = shared_from_this()](const boost::system::error_code& error)
                {
                    onAcceptHandler(error);
                });
            }
        }

        void Acceptor::stop()
        {
            if (running_)
            {
                running_ = false;
                acceptor_.close();
            }
        }

        void Acceptor::setAcceptHandler(AcceptHandler handler)
        {
            handler_ = std::move(handler);
        }

        void Acceptor::onAcceptHandler(const boost::system::error_code& error)
        {
            if (!error)
            {
                handler_(std::move(socket_));
                acceptor_.async_accept(socket_, [this, self = shared_from_this()](const boost::system::error_code& error)
                {
                    onAcceptHandler(error);
                });
            }
        }
    }
}