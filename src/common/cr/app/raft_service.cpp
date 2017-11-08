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

namespace cr
{
    namespace app
    {

        RaftService::RaftService(cr::app::Application& context, boost::asio::io_service& ioService,
            std::uint32_t id, std::string name, const Options& options)
            : cr::app::Service(context, ioService, id, name),
            ioService_(ioService),
            logger_(name),
            options_(options),
            random_(std::random_device()()),
            rocksdb_(nullptr),
            tickTimer_(ioService_),
            acceptor_(ioService_),
            socket_(ioService_),
            connectTimer_(ioService_)
        {
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
                    servers_.push_back(boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::address::from_string(myNode.host().to_string()),
                        boost::lexical_cast<std::uint16_t>(myNode.port().to_string())));
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
            auto listenIp = boost::asio::ip::address::from_string(myNode.host().to_string());
            auto listenPort = boost::lexical_cast<std::uint16_t>(myNode.port().to_string());
            acceptor_.non_blocking(true);
            acceptor_.bind(boost::asio::ip::tcp::endpoint(listenIp, listenPort));
        }

        RaftService::~RaftService()
        {}

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
            // 开始raft
            firstRunRaft();
            // 开始监听
            firstRunAccept();
            // 连接客户端
            firstRunConnect();
        }

        void RaftService::onStop()
        {
            cr::app::Service::onStop();
            // 停止raft
            shutdownRaft();
            // 停止监听
            shutdownPeers();
            // 停止客户端
            shutdownClients();
        }

        void RaftService::onLeaderConnected()
        {}

        void RaftService::onLeaderDisconnected()
        {}

        bool RaftService::isLeaderConnected()
        {
            auto& state = raft_->getState();
            return !state.isCandidate() && raft_->getLeaderId() != boost::none;
        }

        bool RaftService::propose(const std::vector<std::string>& values)
        {
            if (raft_->getState().isLeader())
            {
                raft_->propose(values);
                runRaft();
                return true;
            }
            if (raft_->getLeaderId() != boost::none)
            {
                // 构造消息
                auto message = std::make_shared<cr::raft::pb::RaftMsg>();
                message->set_from_node_id(options_.myId);
                message->set_dest_node_id(raft_->getLeaderId().get());
                // 提交数据
                auto proposeReq = message->mutable_propose_req();
                proposeReq->set_version(0);
                for (auto&& value : values)
                {
                    proposeReq->add_values(value);
                }
                // 发送
                return sendRaftMsg(message);
            }
            return false;
        }

        void RaftService::firstRunRaft()
        {
            auto timePoint = std::chrono::steady_clock::now();
            auto nowTime = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
            raft_->start(nowTime);
            runRaft();
        }

        void RaftService::runRaft()
        {
            auto timePoint = std::chrono::steady_clock::now();
            auto nowTime = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()).count());
            std::uint64_t nextTime = nowTime;
            // 执行逻辑update
            for (std::size_t i = 0; i != 10 && nextTime <= nowTime; ++i)
            {
                nextTime = raft_->update(nowTime, messages_);
            }
            // 运行状态机
            while (raft_->execute()) continue;
            // 发送消息
            for (auto&& message : messages_)
            {
                sendRaftMsg(message);
            }
            messages_.clear();
            // 投递定时器
            runRaftTimer(nextTime);
        }

        void RaftService::onRaftTimerHandler()
        {
            runRaft();
        }

        void RaftService::runRaftTimer(std::uint64_t expiresTime)
        {
            auto expiresAt = std::chrono::steady_clock::time_point(std::chrono::milliseconds(expiresTime));
            if (expiresAt > tickTimer_.expires_at())
            {
                tickTimer_.expires_at(expiresAt);
                tickTimer_.async_wait([this](const boost::system::error_code& error)
                {
                    if (!error)
                    {
                        onRaftTimerHandler();
                    }
                });
            }
        }

        void RaftService::shutdownRaft()
        {
            tickTimer_.cancel();
        }

        void RaftService::firstRunAccept()
        {
            acceptor_.listen();
            runAccept();
        }

        void RaftService::onAcceptHandler()
        {
            cr::network::PbConnection::Options options;
            auto connection = std::make_shared<cr::network::PbConnection>(std::move(socket_), logger_, options);
            connection->setMessageHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message)
            {
                onMessageHandler(conn, message);
            });
            connection->setCloseHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn)
            {
                onPeerDisconectHandler(conn);
            });
            peers_.push_back(connection);
            connection->start();
        }

        void RaftService::runAccept()
        {
            acceptor_.async_accept(socket_, [this](const boost::system::error_code& error)
            {
                if (!error)
                {
                    onAcceptHandler();
                    runAccept();
                }
            });
        }

        void RaftService::onPeerDisconectHandler(const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            auto peerEndpoint = conn->getRemoteEndpoint();
            CRLOG_DEBUG(logger_, "RaftService") << "Peer Socket Disconnect: " << peerEndpoint.address().to_string() << ":" << peerEndpoint.port();
            peers_.erase(std::remove(peers_.begin(), peers_.end(), conn), peers_.end());
        }

        void RaftService::shutdownPeers()
        {
            acceptor_.cancel();
            for (auto&& conn : peers_)
            {
                conn->close();
            }
            peers_.clear();
        }

        void RaftService::firstRunConnect()
        {
            for (auto&& endpoint : servers_)
            {
                connectors_.push_back(std::make_unique<boost::asio::ip::tcp::socket>(ioService_));
                clients_.emplace_back(nullptr);
            }
            for (std::size_t index = 0; index != connectors_.size(); ++index)
            {
                auto& connector = connectors_[index];
                connector->open(servers_[index].protocol());
                runConnect(index);
            }
        }

        void RaftService::onConnectHandler(std::size_t index)
        {
            cr::network::PbConnection::Options options;
            auto& socket = *connectors_[index];
            auto connection = std::make_shared<cr::network::PbConnection>(std::move(socket), logger_, options);
            connection->setMessageHandler([this, self = shared_from_this()](const std::shared_ptr<cr::network::PbConnection>& conn,
                const std::shared_ptr<google::protobuf::Message>& message)
            {
                onMessageHandler(conn, message);
            });
            connection->setCloseHandler([this, self = shared_from_this(), index](const std::shared_ptr<cr::network::PbConnection>& conn)
            {
                onClientDisconnectHandler(index, conn);
            });
            clients_[index] = connection;
            connection->start();
        }

        void RaftService::runConnect(std::size_t index)
        { 
            auto& connector = connectors_[index];
            connector->async_connect(servers_[index], [this, self = shared_from_this(), index](const boost::system::error_code& error)
            {
                if (error && !connectors_.empty())
                {
                    boost::system::error_code ignored;
                    connectors_[index]->close(ignored);
                    runConnectTimer();
                }
                else if (!error)
                {
                    onConnectHandler(index);
                }
            });
        }

        void RaftService::onConnectTimerHandler()
        {
            for (std::size_t index = 0; index != clients_.size(); ++index)
            {
                if (clients_[index] == nullptr)
                {
                    runConnect(index);
                }
            }
        }

        void RaftService::runConnectTimer()
        {
            bool needTimer = false;
            // 打开socket
            for (std::size_t index = 0; index != connectors_.size(); ++index)
            {
                if (!connectors_[index]->is_open() && clients_[index] == nullptr)
                {
                    connectors_[index]->open(servers_[index].protocol());
                    needTimer = true;
                }
            }
            // 投递定时器
            if (needTimer)
            {
                connectTimer_.expires_from_now(std::chrono::seconds(1));
                connectTimer_.async_wait([this](const boost::system::error_code& error)
                {
                    if (!error)
                    {
                        onConnectTimerHandler();
                    }
                });
            }
        }

        void RaftService::onClientDisconnectHandler(std::size_t index, const std::shared_ptr<cr::network::PbConnection>& conn)
        {
            clients_[index].reset();
            runConnectTimer();
        }

        void RaftService::shutdownClients()
        {
            connectTimer_.cancel();
            for (auto&& conn : clients_)
            {
                if (conn)
                {
                    conn->close();
                }
            }
            connectors_.clear();
            clients_.clear();
        }

        void RaftService::onMessageHandler(const std::shared_ptr<cr::network::PbConnection>& conn,
            const std::shared_ptr<google::protobuf::Message>& message)
        {
            if (message->GetDescriptor() != cr::raft::pb::RaftMsg::descriptor())
            {
                CRLOG_WARN(logger_, "RaftService") << "Unknow Message: " << message->GetTypeName();
                return;
            }
            onRaftMsgHandler(std::static_pointer_cast<cr::raft::pb::RaftMsg>(message));
        }

        void RaftService::onRaftMsgHandler(const std::shared_ptr<cr::raft::pb::RaftMsg>& message)
        {
            if (message->dest_node_id() != options_.myId)
            {
                CRLOG_WARN(logger_, "RaftService") << "Unknow Raft Node: " << message->dest_node_id();
                return;
            }
            if (message->has_propose_req())
            {
                CRLOG_DEBUG(logger_, "RaftService") << "Handle Propose From Node: " << message->from_node_id();
                onProposeHandler(message);
            }
            else if (message->has_propose_resp())
            {
                CRLOG_DEBUG(logger_, "RaftService") << "Handle Propose Resp From Node: " << message->from_node_id();
            }
            else
            {
                CRLOG_DEBUG(logger_, "RaftService") << "Handle Message {" << message->body_case() << "} From Node: " << message->from_node_id();
                raft_->receive(message);
                runRaft();
            }
        }

        void RaftService::onProposeHandler(const std::shared_ptr<cr::raft::pb::RaftMsg>& message)
        {
            auto& proposeReq = message->propose_req();
            bool success = false;
            if (raft_->getState().isLeader())
            {
                std::vector<std::string> values;
                std::copy(proposeReq.values().begin(), proposeReq.values().end(), std::back_inserter(values));
                // 执行raft逻辑
                success = propose(values);
            }
            // 回复
            auto response = std::make_shared<cr::raft::pb::RaftMsg>();
            response->set_from_node_id(options_.myId);
            response->set_dest_node_id(message->from_node_id());
            auto proposeResp = response->mutable_propose_resp();
            proposeResp->set_version(proposeReq.version());
            proposeResp->set_success(success);
            // 发送
            sendRaftMsg(response);
        }

        bool RaftService::sendRaftMsg(const std::shared_ptr<cr::raft::pb::RaftMsg>& message)
        {
            auto& buddyNodeIds = raft_->getBuddyNodeIds();
            for (std::size_t index = 0; index != buddyNodeIds.size(); ++index)
            {
                if (buddyNodeIds[index] == message->dest_node_id())
                {
                    if (clients_[index] != nullptr)
                    {
                        clients_[index]->send(message);
                        return true;
                    }
                    return false;
                }
            }
            return false;
        }
    }
}