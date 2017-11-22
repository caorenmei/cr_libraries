#include "service_options.h"

#include "exception.h"

namespace cr
{
    namespace raft
    {
        ServiceOptions::ServiceOptions()
            : myId_(0),
            minElectionTime_(500),
            maxElectionTime_(1000),
            heartbeatTime_(100),
            binLogPath_()
        {}

        ServiceOptions::~ServiceOptions()
        {}

        void ServiceOptions::setMyId(std::uint64_t myId)
        {
            myId_ = myId;
        }

        std::uint64_t ServiceOptions::getMyId() const
        {
            return myId_;
        }

        void ServiceOptions::setBinLogPath(std::string binLogPath)
        {
            binLogPath_ = std::move(binLogPath);
        }

        const std::string& ServiceOptions::getBinLogPath() const
        {
            return binLogPath_;
        }

        const boost::asio::ip::tcp::endpoint& ServiceOptions::getMyEndpoint() const
        {
            for (auto& node : nodes_)
            {
                if (node.first == myId_)
                {
                    return node.second;
                }
            }
            throw ConfigException();
        }

        void ServiceOptions::addNode(std::uint64_t id, const boost::asio::ip::tcp::endpoint& endpoint)
        {
            nodes_.emplace_back(id, endpoint);
        }

        const std::vector<std::pair<std::uint64_t, boost::asio::ip::tcp::endpoint>>& ServiceOptions::getNodes() const
        {
            return nodes_;
        }

        std::vector<std::uint64_t> ServiceOptions::getBuddyNodeIds() const
        {
            std::vector<std::uint64_t> nodeIds;
            for (auto& node : nodes_)
            {
                if (node.first != myId_)
                {
                    nodeIds.push_back(node.first);
                }
            }
            return nodeIds;
        }

        std::vector<std::pair<std::uint64_t, boost::asio::ip::tcp::endpoint>> ServiceOptions::getBuddyNodes() const
        {
            std::vector<std::pair<std::uint64_t, boost::asio::ip::tcp::endpoint>> buddies;
            for (auto& node : nodes_)
            {
                if (node.first != myId_)
                {
                    buddies.push_back(node);
                }
            }
            return buddies;
        }

        void ServiceOptions::setElectionTime(std::uint64_t minElectionTime, std::uint64_t maxElectionTime)
        {
            minElectionTime_ = minElectionTime;
            maxElectionTime_ = maxElectionTime;
        }

        std::uint64_t ServiceOptions::getMinElectionTime() const
        {
            return minElectionTime_;
        }

        std::uint64_t ServiceOptions::getMaxElectionTime() const
        {
            return maxElectionTime_;
        }

        void ServiceOptions::setHeartbeatTime(std::uint64_t heartbeatTime)
        {
            heartbeatTime_ = heartbeatTime;
        }

        std::uint64_t ServiceOptions::getHeartbeatTime() const
        {
            return heartbeatTime_;
        }
    }
}