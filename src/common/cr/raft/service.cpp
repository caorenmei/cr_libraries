#include "service.h"

#include <cassert>
#include <algorithm>
#include <random>

#include <boost/filesystem.hpp>
#include <rocksdb/options.h>

#include <cr/raft/file_storage.h>
#include <cr/raft/raft_msg.pb.h>

#include "service.pb.h"

namespace cr
{
    namespace raft
    {

        Service::Service(cr::app::Application& context, boost::asio::io_service& ioService,
            std::uint32_t id, std::string name, const Options& options)
            : cr::app::Service(context, ioService, id, name),
            ioService_(ioService),
            logger_(name),
            options_(options),
            random_(std::random_device()()),
            rocksdb_(nullptr),
            timer_(ioService_),
            acceptor_(ioService_),
            socket_(ioService_)
        {
            // 姐弟Id
            std::vector<std::uint64_t> buddyIds;
            for (auto& server : options_.getNodes())
            {
                if (server.first == options_.getMyId())
                {
                    server_ = server.second;
                }
                else
                {
                    buddyIds.push_back(server.first);
                }
            }
            // 创建目录
            boost::filesystem::create_directories(options_.getBinLogPath());
            // 打开db
            rocksdb::Options dbOptions;
            dbOptions.create_if_missing = true;
            rocksdb::DB* db = nullptr;
            auto status = rocksdb::DB::Open(dbOptions, options_.getBinLogPath(), &db);
            assert(status.ok());
            rocksdb_ = std::unique_ptr<rocksdb::DB>(db);
            // storage
            auto storage = std::make_shared<cr::raft::FileStorage>(rocksdb_.get(), logger_);
            // options
            cr::raft::Options raftOptons;
            raftOptons.setNodeId(options_.getMyId())
                .setBuddyNodeIds(buddyIds)
                .setElectionTimeout(options_.getMinElectionTime(), options_.getMaxElectionTime())
                .setHeartbeatTimeout(options_.getHeartbeatTime())
                .setRandomSeed(random_())
                .setStorage(storage)
                .setMaxWaitEntriesNum(64)
                .setMaxPacketSize(64 * 1024);
            raftOptons.setEexcutable([this](std::uint64_t index, const std::string& value)
            {
                onCommand(index, value);
            });
            raft_ = std::make_unique<cr::raft::Raft>(raftOptons);
            // 创建节点
            for (auto& server : options_.getNodes())
            {
                auto node = std::make_shared<RaftNode>(ioService_, logger_, *raft_, server.first, server.second);
                node->setConnectCallback([this](const std::shared_ptr<RaftNode>& node)
                {
                    onNodeConnectHandler(node);
                });
                node->setUpdateCallback([this](const std::shared_ptr<RaftNode>& node)
                {
                    update();
                });
                nodes_.insert(std::make_pair(server.first, node));
            }    
            // 编解码器
            codec_.setMessageHandler([this](const std::shared_ptr<cr::network::Connection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message)
            {
                onMessageHandler(conn, message);
            });
            codec_.setErrorHandler([this](const std::shared_ptr<cr::network::Connection>& conn, int)
            {
                onMessageErrorHandler(conn);
            });
        }

        Service::~Service()
        {}

        cr::log::Logger& Service::getLogger()
        {
            return logger_;
        }

        const Service::Options& Service::getOptions() const
        {
            return options_;
        }

        void Service::onStart()
        {
            cr::app::Service::onStart();
            // 开始监听
            acceptor_.non_blocking(true);
            acceptor_.bind(server_);
            acceptor_.listen();
            accept();
            // 启动节点
            for (auto& node : nodes_)
            {
                node.second->start();
            }
            // 开始逻辑
            update();
        }

        void Service::onStop()
        {
            cr::app::Service::onStop();
            // 停止监听
            acceptor_.close();
            //关闭连接
            for (auto& conn : connections_)
            {
                conn.first->close();
            }
            connections_.clear();
            // 停止节点
            for (auto& node : nodes_)
            {
                node.second->stop();
            }
            nodes_.clear();
            // 停止逻辑
            timer_.cancel();
        }

        void Service::onLeaderConnected()
        {}

        void Service::onLeaderDisconnected()
        {}

        bool Service::isLeaderConnected()
        {
            auto leaderId = raft_->getLeaderId();
            if (leaderId.is_initialized())
            {
                auto nodeIter = nodes_.find(*leaderId);
                assert(nodeIter != nodes_.end());
                return nodeIter->second->isConnected();
            }
            return false;
        }

		const cr::raft::IRaftState& Service::getState() const
		{
			return raft_->getState();
		}

        bool Service::execute(const std::string& value, std::function<void(std::uint64_t, int)> cb)
        {
            auto leaderId = raft_->getLeaderId();
            if (leaderId.is_initialized())
            {
                auto nodeIter = nodes_.find(*leaderId);
                assert(nodeIter != nodes_.end());
                return nodeIter->second->execute(value, std::move(cb));
            }
            return false;
        }

        void Service::accept()
        {
            acceptor_.async_accept(socket_, [this](const boost::system::error_code& error)
            {
                if (!error)
                {
                    onConnectHandler();
                    accept();
                }
            });
        }

        void Service::onConnectHandler()
        {
            auto conn = std::make_shared<cr::network::Connection>(std::move(socket_));
            onConnectHandler(conn);
        }

        void Service::onConnectHandler(const std::shared_ptr<cr::network::Connection>& conn)
        {
            auto peerEndpoint = conn->getRemoteEndpoint();
            CRLOG_DEBUG(logger_, "Service") << "Peer Socket Disconnect: " << peerEndpoint.address().to_string() << ":" << peerEndpoint.port();
            conn->setMessageHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::Connection>& conn,
                cr::network::ByteBuffer& buffer)
            {
                codec_.onMessage(conn, buffer);
            });
            conn->setCloseHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::Connection>& conn)
            {
                onDisconectHandler(conn);
            });
            connections_.insert(std::make_pair(conn, nullptr));
            conn->start();
        }

        void Service::onDisconectHandler(const std::shared_ptr<cr::network::Connection>& conn)
        {
            auto peerEndpoint = conn->getRemoteEndpoint();
            CRLOG_DEBUG(logger_, "Service") << "Peer Socket Disconnect: " << peerEndpoint.address().to_string() << ":" << peerEndpoint.port();
            auto connIter = connections_.find(conn);
            if (connIter != connections_.end())
            {
                if (connIter->second)
                {
                    connIter->second->onDisconnect(conn);
                }
                connections_.erase(connIter);
            }
        }

        void Service::onMessageHandler(const std::shared_ptr<cr::network::Connection>& conn,
            const std::shared_ptr<google::protobuf::Message>& message)
        {
            if (message->GetDescriptor() == pb::RaftHandshakeReq::descriptor())
            {
                onMessageHandler(conn, std::static_pointer_cast<pb::RaftHandshakeReq>(message));
                return;
            }
            auto connIter = connections_.find(conn);
            if (connIter != connections_.end() && connIter->second)
            {
                connIter->second->onMessage(message);
            }
        }

        void Service::onMessageErrorHandler(const std::shared_ptr<cr::network::Connection>& conn)
        {
            conn->close();
        }

        void Service::onMessageHandler(const std::shared_ptr<cr::network::Connection>& conn, const std::shared_ptr<pb::RaftHandshakeReq>& message)
        {
            auto connIter = connections_.find(conn);
            if (connIter != connections_.end())
            {
                return;
            }
            bool success = false;
            do
            {
                // 已经握手
                if (connIter->second)
                {
                    break;
                }
                // 节点不匹配
                if (message->from_node_id() == message->dest_node_id() && message->dest_node_id() != options_.getMyId())
                {
                    break;
                }
                // 节点没找到
                auto nodeIter = nodes_.find(message->from_node_id());
                if (nodeIter == nodes_.end())
                {
                    break;
                }
                // 握手成功
                connIter->second = nodeIter->second;
                success = true;
            } while (0);
            // 回复
            pb::RaftHandshakeResp response;
            response.set_success(success);
            codec_.send(conn, response);
            // 回调连接成功
            if (success)
            {
                connIter->second->onConnect(conn);
            }
            // 关闭连接
            else
            {
                conn->close();
            }
        }

        void Service::onNodeConnectHandler(const std::shared_ptr<RaftNode>& node)
        {
            if (node->isLeader())
            {
                if (node->isConnected())
                {
                    onLeaderConnected();
                }
                else if (!node->isConnected())
                {
                    onLeaderDisconnected();
                }
            }
        }

        void Service::update()
        {
            auto timePoint = std::chrono::steady_clock::now();
            auto nowTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
            // 之前的领导节点状态
            auto leaderId = raft_->getLeaderId();
            bool connected = false;
            if (leaderId.is_initialized())
            {
                auto nodeIter = nodes_.find(*leaderId);
                assert(nodeIter != nodes_.end());
                connected = nodeIter->second->isConnected();
            }
            // 更新状态机
            auto nextTime = raft_->update(nowTime.count(), messages_);
            // 当前的节点
            auto nowLeaderId = raft_->getLeaderId();
            auto nowConnected = false;
            if (nowLeaderId.is_initialized() && nowLeaderId != leaderId)
            {
                auto nodeIter = nodes_.find(*nowLeaderId);
                assert(nodeIter != nodes_.end());
                nowConnected = nodeIter->second->isConnected();
            }
            // 丢失了领导者
            if ((leaderId.is_initialized() && connected) && (leaderId != nowLeaderId))
            {
                onLeaderDisconnected();
                if (nowLeaderId.is_initialized() && nowConnected)
                {
                    onLeaderConnected();
                }
            }
            // 成为领导者
            else if (!(leaderId.is_initialized() && connected) && (nowLeaderId.is_initialized() && nowConnected))
            {
                onLeaderConnected();
            }
            // 发送消息
            for (auto&& message : messages_)
            {
                auto nodeIter = nodes_.find(message->dest_node_id());
                assert(nodeIter != nodes_.end());
                nodeIter->second->sendMessage(message);
            }
            messages_.clear();
            // 投递定时器
            timer_.expires_from_now(std::chrono::milliseconds(nextTime - nowTime.count()));
            timer_.async_wait([this](const boost::system::error_code& error)
            {
                if (!error)
                {
                    update();
                }
            });
        }
    }
}