#include "raft_service.h"

#include <cassert>
#include <algorithm>
#include <random>

#include <boost/filesystem.hpp>
#include <network/uri.hpp>
#include <boost/lexical_cast.hpp>
#include <rocksdb/options.h>

#include <cr/core/streams.h>
#include <cr/raft/file_storage.h>
#include <cr/raft/raft_msg.pb.h>

#include "raft_service.pb.h"

namespace cr
{
    namespace app
    {

        template <typename T, typename Alloctor>
        static std::size_t vectorIndexOf(const std::vector<T, Alloctor>& v, const T& value)
        {
            auto iter = std::find(v.begin(), v.end(), value);
            return iter - v.begin();
        }

        RaftService::RaftService(cr::app::Application& context, boost::asio::io_service& ioService,
            std::uint32_t id, std::string name, const Options& options)
            : cr::app::Service(context, ioService, id, name),
            ioService_(ioService),
            logger_(name),
            options_(options),
            random_(std::random_device()()),
            rocksdb_(nullptr),
            version_(0),
            timer_(ioService_),
            acceptor_(ioService_),
            socket_(ioService_)
        {
            // 解析地址
            auto parseUri = [](const ::network::uri& uri)
            {
                auto ip = boost::asio::ip::address::from_string(uri.host().to_string());
                auto port = boost::lexical_cast<std::uint16_t>(uri.port().to_string());
                return boost::asio::ip::tcp::endpoint(ip, port);
            };
            // 解析node
            ::network::uri myNode;
            std::vector<std::uint64_t> buddyNodeIds;
            for (auto& server : options_.servers)
            {
                ::network::uri uri(server);
                // 解析Id
                auto path = uri.path().to_string();
                auto id = boost::lexical_cast<std::uint64_t>(path.substr(1));
                // 自身节点
                if (id != options_.myId)
                {
                    buddyNodeIds.push_back(id);
                    connectors_.push_back(std::make_shared<cr::network::Connector>(ioService_, parseUri(uri)));
                }
                else
                {
                    myNode = uri;
                }
            }
            assert(!myNode.empty());
            // 创建目录
            boost::filesystem::create_directories(options_.binLogPath);
            // 打开db
            rocksdb::Options dbOptions;
            dbOptions.create_if_missing = true;
            auto status = rocksdb::DB::Open(dbOptions, options_.binLogPath, &rocksdb_);
            assert(status.ok());
            // storage
            auto storage = std::make_shared<cr::raft::FileStorage>(rocksdb_, logger_);
            // options
            cr::raft::Options raftOptons;
            raftOptons.setNodeId(options_.myId)
                .setBuddyNodeIds(buddyNodeIds)
                .setElectionTimeout(options_.minElectionTime, options_.maxElectionTime)
                .setHeartbeatTimeout(options_.heatbeatTime)
                .setRandomSeed(random_())
                .setStorage(storage)
                .setMaxWaitEntriesNum(64)
                .setMaxPacketSize(64 * 1024);
            raftOptons.setEexcutable([this](std::uint64_t index, const std::string& value)
            {
                onCommand(index, value);
            });
            raft_ = std::make_unique<cr::raft::Raft>(raftOptons);
            // 监听socket
            acceptor_.non_blocking(true);
            acceptor_.bind(parseUri(myNode));
        }

        RaftService::~RaftService()
        {
            delete rocksdb_;
        }

        cr::log::Logger& RaftService::getLogger()
        {
            return logger_;
        }

        const RaftService::Options& RaftService::getOptions() const
        {
            return options_;
        }

        void RaftService::onStart()
        {
            cr::app::Service::onStart();
            // 开始监听
            acceptor_.listen();
            accept();
            // 连接客户端
            connect();
        }

        void RaftService::onStop()
        {
            cr::app::Service::onStop();
        }

        void RaftService::onLeaderConnected()
        {}

        void RaftService::onLeaderDisconnected()
        {}

        bool RaftService::isLeaderConnected()
        {
            auto& state = raft_->getState();
            auto leaderId = raft_->getLeaderId();
            if (state.isFollower() && leaderId)
            {
                auto leaderIndex = vectorIndexOf(raft_->getBuddyNodeIds(), *leaderId);
                return connections_[leaderIndex] != nullptr;
            }
            if (state.isLeader())
            {
                return true;
            }
            return false;
        }

		const cr::raft::IRaftState& RaftService::getState() const
		{
			return raft_->getState();
		}

        bool RaftService::execute(const std::string& value, std::function<void(std::uint64_t, int)> cb)
        {
            auto& state = raft_->getState();
            // 领导者，直接提交任务
            if (state.isLeader())
            {
                raft_->execute(value, std::move(cb));
                update();
                return true;
            }
            // 领导者已连接
            if (isLeaderConnected())
            {
                // 领导者
                auto leaderId = raft_->getLeaderId();
                auto leaderIndex = vectorIndexOf(raft_->getBuddyNodeIds(), leaderId.get());
                // 保存回调
                version_ = version_ + 1;
                callbacks_[leaderIndex].insert(std::make_pair(version_, std::move(cb)));
                // 构造消息
                auto message = std::make_shared<cr::raft::pb::RaftMsg>();
                message->set_from_node_id(options_.myId);
                message->set_dest_node_id(leaderId.get());
                // 数据请求
                auto request = message->mutable_propose_req();
                request->set_version(version_);
                request->set_value(value);
                // 发送
                connections_[leaderIndex]->send(message);
                // 完成
                return true;
            }
            return false;
        }

        void RaftService::accept()
        {
            acceptor_.async_accept(socket_, [this](const boost::system::error_code& error)
            {
                if (!error)
                {
                    onAcceptHandler();
                    accept();
                }
            });
        }

        void RaftService::onAcceptHandler()
        {
            cr::network::PbConnection::Options options;
            auto connection = std::make_shared<cr::network::PbConnection>(std::move(socket_), logger_, options);
            connection->setMessageHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message)
            {
                if (message->GetDescriptor() == cr::raft::pb::RaftMsg::descriptor())
                {
                    onPeerMessageHandler(conn, std::static_pointer_cast<cr::raft::pb::RaftMsg>(message));
                }
                else if (message->GetDescriptor() == cr::app::pb::RaftHandshakeReq::descriptor())
                {
                    onPeerMessageHandler(conn, std::static_pointer_cast<cr::app::pb::RaftHandshakeReq>(message));
                }
                else if (message->GetDescriptor() == cr::app::pb::RaftProposeReq::descriptor())
                {
                    onPeerMessageHandler(conn, std::static_pointer_cast<cr::app::pb::RaftProposeReq>(message));
                }
                else if (message->GetDescriptor() == cr::app::pb::RaftProposeResp::descriptor())
                {
                    onPeerMessageHandler(conn, std::static_pointer_cast<cr::app::pb::RaftProposeResp>(message));
                }
            });
            connection->setCloseHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn)
            {
                onPeerDisconectedHandler(conn);
            });
            peers_.insert(connection);
            connection->start();
        }

        void RaftService::onPeerDisconectedHandler(const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            auto peerEndpoint = conn->getRemoteEndpoint();
            CRLOG_DEBUG(logger_, "RaftService") << "Peer Socket Disconnect: " << peerEndpoint.address().to_string() << ":" << peerEndpoint.port();
            peers_.erase(conn);
            // 从连接移除
            auto connIndex = vectorIndexOf(connections_, conn);
            if (connIndex < connections_.size())
            {
                connections_[connIndex] = nullptr;
                onDisconnect(connIndex);
            }
        }

        void RaftService::connect()
        {
            auto& buddyIds = raft_->getBuddyNodeIds();
            for (std::size_t index = 0; index != buddyIds.size(); ++index)
            {
                if (buddyIds[index] < options_.myId)
                {
                    connectors_[index]->start([this, index](boost::asio::ip::tcp::socket socket)
                    {
                        cr::network::PbConnection::Options options;
                        auto conn = std::make_shared<cr::network::PbConnection>(std::move(socket), logger_, options);
                        onClientConnectedHandler(index, conn);
                    });
                }
            }
        }

        void RaftService::connect(std::size_t index)
        {
            connectors_[index]->delayStart([this, index](boost::asio::ip::tcp::socket socket)
            {
                cr::network::PbConnection::Options options;
                auto conn = std::make_shared<cr::network::PbConnection>(std::move(socket), logger_, options);
                onClientConnectedHandler(index, conn);
            });
        }

        void RaftService::onClientConnectedHandler(std::size_t index, const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            if (index < connections_.size())
            {
                // 消息处理器
                conn->setMessageHandler([this, self = shared_from_this(), index](const std::shared_ptr<cr::network::PbConnection>& conn,
                    const std::shared_ptr<google::protobuf::Message>& message)
                {
                    if (message->GetDescriptor() == cr::raft::pb::RaftMsg::descriptor())
                    {
                        onMessageHandler(index, std::static_pointer_cast<cr::raft::pb::RaftMsg>(message));
                    }
                    else if (message->GetDescriptor() == cr::app::pb::RaftHandshakeResp::descriptor())
                    {
                        onMessageHandler(index, std::static_pointer_cast<cr::app::pb::RaftHandshakeResp>(message));
                    }
                    else if (message->GetDescriptor() == cr::app::pb::RaftProposeReq::descriptor())
                    {
                        onMessageHandler(index, std::static_pointer_cast<cr::app::pb::RaftProposeReq>(message));
                    }
                    else if (message->GetDescriptor() == cr::app::pb::RaftProposeResp::descriptor())
                    {
                        onMessageHandler(index, std::static_pointer_cast<cr::app::pb::RaftProposeResp>(message));
                    }
                });
                // 关闭处理器
                conn->setCloseHandler([this, self = shared_from_this(), index](const std::shared_ptr<cr::network::PbConnection>& conn)
                {
                    onClientDisconnectHandler(index, conn);
                });
                // 保存连接
                connections_[index] = conn;
                // 开始
                conn->start();
                // 构造握手消息
                auto& buddyIds = raft_->getBuddyNodeIds();
                auto request = std::make_shared<cr::raft::pb::RaftMsg>();
                request->set_from_node_id(options_.myId);
                request->set_dest_node_id(buddyIds[index]);
                request->mutable_handshake_req();
                // 发送
                conn->send(request);
            }
        }

        void RaftService::onClientDisconnectHandler(std::size_t index, const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            if (index < connections_.size())
            {
                // 清除连接
                connections_[index] = nullptr;
                // 重连
                connect(index);
                // 回调
                onDisconnect(index);
            }
        }

        void RaftService::onPeerMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
            const std::shared_ptr<cr::raft::pb::RaftMsg>& message)
        {
            // 已经握手成功
            auto connIter = std::find(connections_.begin(), connections_.end(), conn);
            if (connIter != connections_.end())
            {
                std::size_t index = connIter - connections_.begin();
                onMessageHandler(index, message);
                return;
            }
            // 处理的第一个消息为握手消息
            if (!message->has_handshake_req())
            {
                CRLOG_WARN(logger_, "RaftService") << "First Message Is Not HandshakeReq: MyId=" << options_.myId;
                conn->close();
                return;
            }
            onPeerMessageHandler(conn, message);
        }

        void RaftService::onPeerMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
            const std::shared_ptr<cr::app::pb::RaftHandshakeReq>& message)
        {
            // 判断握手条件
            bool success = false;
            do
            {
                if (message->dest_node_id() != options_.myId)
                {
                    CRLOG_WARN(logger_, "RaftService") << "Handshake Failed: MyId=" << options_.myId << ", From Node Id=" << message->dest_node_id();
                    break;
                }
                if (message->from_node_id() <= options_.myId)
                {
                    CRLOG_WARN(logger_, "RaftService") << "Handshake Failed: MyId=" << options_.myId << ", From Node Id=" << message->dest_node_id();
                    break;
                }
                auto& buddyIds = raft_->getBuddyNodeIds();
                if (std::find(buddyIds.begin(), buddyIds.end(), message->from_node_id()) == buddyIds.end())
                {
                    CRLOG_WARN(logger_, "RaftService") << "Handshake Failed: MyId=" << options_.myId << ", From Node Id=" << message->dest_node_id();
                    break;
                }
                CRLOG_INFO(logger_, "RaftService") << "Handshake Success: MyId=" << options_.myId << ", Frm Node Id=" << message->from_node_id();
                success = true;
            } while (0);
            // 应答消息
            auto response = std::make_shared<cr::raft::pb::RaftMsg>();
            response->set_from_node_id(options_.myId);
            response->set_dest_node_id(message->from_node_id());
            response->mutable_handshake_resp()->set_success(success);
            // 回复
            conn->send(response);
            // 握手成功, 保存连接
            if (success)
            {
                auto index = vectorIndexOf(raft_->getBuddyNodeIds(), message->from_node_id());
                connections_[index] = conn;
            }
            // 握手失败，关闭连接
            else
            {
                conn->close();
            }
        }

        void RaftService::onPeerMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
            const std::shared_ptr<cr::app::pb::RaftProposeReq>& message)
        {
            auto connIter = std::find(connections_.begin(), connections_.end(), conn);
            if (connIter != connections_.end())
            {
                std::size_t index = connIter - connections_.begin();
                onMessageHandler(index, message);
                return;
            }
        }

        void RaftService::onPeerMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
            const std::shared_ptr<cr::app::pb::RaftProposeResp>& message)
        {
            auto connIter = std::find(connections_.begin(), connections_.end(), conn);
            if (connIter != connections_.end())
            {
                std::size_t index = connIter - connections_.begin();
                onMessageHandler(index, message);
                return;
            }
        }

        void RaftService::onMessageHandler(std::size_t index, const std::shared_ptr<cr::raft::pb::RaftMsg>& message)
        {
            // 提交消息
            raft_->receive(message);
        }

        void RaftService::onMessageHandler(std::size_t index, const std::shared_ptr<cr::app::pb::RaftHandshakeResp>& message)
        {
            if (!message->success())
            {
                connections_[index]->close();
            }
        }

        void RaftService::onMessageHandler(std::size_t index, const std::shared_ptr<cr::app::pb::RaftProposeReq>& message)
        {
            std::weak_ptr<cr::network::PbConnection> conn = connections_[index];
            // 执行回调
            auto callback = [this, conn, message](std::uint64_t logIndex, int reason)
            {
                auto connection = conn.lock();
                if (connection)
                {
                    // 应答消息
                    auto response = std::make_shared<cr::app::pb::RaftProposeResp>();
                    response->set_serial_no(message->serial_no());
                    response->set_result(reason);
                    response->set_index(logIndex);
                    // 发送
                    connection->send(response);
                }
            };
            // 执行
            auto result = raft_->execute(message->value(), callback);
            // 不是领导者
            if (!result.second)
            {
                callback(0, cr::raft::Raft::RESULT_LEADER_LOST);
            }
        }

        void RaftService::onMessageHandler(std::size_t index, const std::shared_ptr<cr::app::pb::RaftProposeResp>& message)
        {
            if (index < callbacks_.size())
            {
                auto& callbacks = callbacks_[index];
                auto cbIter = callbacks.find(message->serial_no());
                if (cbIter != callbacks.end())
                {
                    cbIter->second(message->index(), message->result());
                    callbacks.erase(cbIter);
                }
            }
        }

        void RaftService::update()
        {
            auto timePoint = std::chrono::steady_clock::now();
            auto nowTime = std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch());
            std::uint64_t nextTime = nowTime.count();
            // 执行逻辑update
            auto& state = raft_->getState();
            bool leaderConnected = isLeaderConnected();
            auto nextTime = raft_->update(nowTime.count(), messages_);
            bool nowLeaderConnected = isLeaderConnected();
            // 领导者连接状态
            if (leaderConnected && !nowLeaderConnected)
            {
                onLeaderDisconnected();
            }
            else if (!leaderConnected && nowLeaderConnected)
            {
                onLeaderConnected();
            }
            // 发送消息
            for (auto&& message : messages_)
            {
                auto nodeIndex = vectorIndexOf(raft_->getBuddyNodeIds(), message->dest_node_id());
                if (nodeIndex < connections_.size() && connections_[nodeIndex] != nullptr)
                {
                    connections_[nodeIndex]->send(message);
                }
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