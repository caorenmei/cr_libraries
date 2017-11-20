#include "raft_node.h"

#include "service.pb.h"

namespace cr
{
    namespace raft
    {
        RaftNode::RaftNode(boost::asio::io_service& ioService, cr::log::Logger& logger, Raft& raft, std::uint64_t id,
            const boost::asio::ip::tcp::endpoint& endpoint)
            : ioService_(ioService),
            logger_(logger),
            raft_(raft),
            id_(id),
            endpoint_(endpoint),
            running_(false),
            connected_(raft.getNodeId() == id),
            serialNo_(0)
        {}

        RaftNode::~RaftNode()
        {}

        void RaftNode::setConnectCallback(ConnectCallback cb)
        {
            connectCallback_ = std::move(cb);
        }

        void RaftNode::setUpdateCallback(UpdateCallback cb)
        {
            updateCallback_ = std::move(cb);
        }

        std::uint64_t RaftNode::getId() const
        {
            return id_;
        }

        bool RaftNode::isLeader() const
        {
            auto leaderId = raft_.getLeaderId();
            return leaderId && *leaderId == id_;
        }

        bool RaftNode::isConnected() const
        {
            return connected_;
        }

        void RaftNode::start()
        {
            if (!running_)
            {
                running_ = true;
                if (id_ < raft_.getNodeId())
                {
                    connector_ = std::make_shared<cr::network::Connector>(ioService_, endpoint_);
                    connector_->start([this](boost::asio::ip::tcp::socket socket)
                    {
                        onConnectHandler(std::move(socket));
                    });
                }
            }
        }

        void RaftNode::stop()
        {
            if (running_)
            {
                running_ = false;
                if (connector_ != nullptr)
                {
                    connector_->stop();
                    connector_ = nullptr;
                }
                if (connection_ != nullptr)
                {
                    connection_->close();
                    connection_ = nullptr;
                }
            }
        }

        void RaftNode::onConnect(const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            if (running_ && !connected_)
            {
                connection_ = conn;
                connected_ = true;
                connectCallback_(shared_from_this());
            }
        }

        void RaftNode::onDisconnect(const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            if (running_ && connected_)
            {
                connection_ = nullptr;
                connected_ = false;
                onDisconnect();
            }
        }

        void RaftNode::onMessage(const std::shared_ptr<google::protobuf::Message>& message)
        {
            if (running_)
            {
                onMessageHandler(message);
            }
        }

        void RaftNode::sendMessage(const std::shared_ptr<google::protobuf::Message>& message)
        {
            if (running_ && connected_)
            {
                connection_->send(message);
            }
        }

        bool RaftNode::execute(const std::string& value, std::function<void(std::uint64_t, int)> cb)
        {
            if (running_)
            {
                if (raft_.getState().isLeader())
                {
                    raft_.execute(value, std::move(cb));
                    updateCallback_(shared_from_this());
                    return true;
                }
                else if (isLeader() && connected_)
                {
                    // 保存回调
                    serialNo_ = serialNo_ + 1;
                    callbacks_.insert(std::make_pair(serialNo_, cb));
                    // 转发请求
                    auto request = std::make_shared<cr::raft::pb::RaftProposeReq>();
                    request->set_serial_no(serialNo_);
                    request->set_value(value);
                    connection_->send(request);
                    // 完成
                    return true;
                }
            }
            return false;
        }

        void RaftNode::onConnectHandler(boost::asio::ip::tcp::socket socket)
        {
            if (running_)
            {
                cr::network::PbConnection::Options options;
                auto conn = std::make_shared<cr::network::PbConnection>(std::move(socket), logger_, options);
                onConnectHandler(conn);
            }
        }

        void RaftNode::onConnectHandler(const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            connection_ = conn;
            // 消息处理器
            connection_->setMessageHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message)
            {
                onMessageHandler(message);
            });
            // 关闭处理器
            connection_->setCloseHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn)
            {
                onDisconnectHandler(conn);
            });
            // 开始
            connection_->start();
            // 握手
            auto request = std::make_shared<cr::raft::pb::RaftHandshakeReq>();
            request->set_from_node_id(raft_.getNodeId());
            request->set_dest_node_id(id_);
            connection_->send(request);
        }

        void RaftNode::onDisconnectHandler(const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            if (running_)
            {
                connection_ = nullptr;
                if (connected_)
                {
                    connected_ = false;
                    onDisconnect();
                }
                connector_->delayStart([this](boost::asio::ip::tcp::socket socket)
                {
                    onConnectHandler(std::move(socket));
                });
            }
        }

        void RaftNode::onDisconnect()
        {
            connectCallback_(shared_from_this());
            for (auto& callback : std::move(callbacks_))
            {
                callback.second(0, cr::raft::Raft::RESULT_LEADER_CHANGED);
            }
        }

        void RaftNode::onMessageHandler(const std::shared_ptr<google::protobuf::Message>& message)
        {
            if (running_)
            {
                auto descriptor = message->GetDescriptor();
                // 握手消息
                if (descriptor == cr::raft::pb::RaftHandshakeResp::descriptor())
                {
                    onMessageHandler(std::static_pointer_cast<cr::raft::pb::RaftHandshakeResp>(message));
                }
                // 逻辑消息
                if (connected_)
                {
                    if (descriptor == cr::raft::pb::RaftProposeReq::descriptor())
                    {
                        onMessageHandler(std::static_pointer_cast<cr::raft::pb::RaftProposeReq>(message));
                    }
                    else if (descriptor == cr::raft::pb::RaftProposeResp::descriptor())
                    {
                        onMessageHandler(std::static_pointer_cast<cr::raft::pb::RaftProposeResp>(message));
                    }
                    else if (descriptor == cr::raft::pb::RaftMsg::descriptor())
                    {
                        onMessageHandler(std::static_pointer_cast<cr::raft::pb::RaftMsg>(message));
                    }
                }
            }
        }

        void RaftNode::onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftHandshakeResp>& message)
        {
            if (message->success())
            {
                connected_ = true;
                connectCallback_(shared_from_this());
            }
            else
            {
                connection_->close();
            }
        }

        void RaftNode::onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftProposeReq>& message)
        {
            auto conn = std::weak_ptr<cr::network::PbConnection>(connection_);
            auto callback = [serialNo = message->serial_no(), conn](std::uint64_t index, int reason)
            {
                auto connection = conn.lock();
                if (connection != nullptr)
                {
                    auto response = std::make_shared<cr::raft::pb::RaftProposeResp>();
                    response->set_serial_no(serialNo);
                    response->set_result(reason);
                    response->set_index(index);
                    connection->send(response);
                }
            };
            if (raft_.getState().isLeader())
            {
                raft_.execute(message->value(), callback);
                updateCallback_(shared_from_this());
            }
            else
            {
                callback(0, cr::raft::Raft::RESULT_LEADER_CHANGED);
            }
        }

        void RaftNode::onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftProposeResp>& message)
        {
            auto callbackIter = callbacks_.find(message->serial_no());
            if (callbackIter != callbacks_.end())
            {
                callbackIter->second(message->index(), message->result());
                callbacks_.erase(callbackIter);
            }
        }

        void RaftNode::onMessageHandler(const std::shared_ptr<cr::raft::pb::RaftMsg>& message)
        {
            raft_.receive(message);
            updateCallback_(shared_from_this());
        }
    }
}