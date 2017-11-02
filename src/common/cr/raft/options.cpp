#include "options.h"

namespace cr
{
    namespace raft
    {
        Options::Options()
            : nodeId_(0),
            randomSeed_(0),
            minElectionTimeout_(500),
            maxElectionTimeout_(1000),
            heartbeatTimeout_(200),
            maxWaitEntriesNum_(20),
            maxPacketLength_(1024)
        {}


        Options::~Options()
        {}

        Options& Options::setNodeId(std::uint64_t nodeId)
        {
            nodeId_ = nodeId;
            return *this;
        }

        std::uint64_t Options::getNodeId() const
        {
            return nodeId_;
        }

        Options& Options::setBuddyNodeIds(std::vector<std::uint64_t> otherNodeIds)
        {
            buddyNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        const std::vector<std::uint64_t>& Options::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        Options& Options::setStorage(std::shared_ptr<Storage> storage)
        {
            storage_ = std::move(storage);
            return *this;
        }

        const std::shared_ptr<Storage>& Options::getStorage() const
        {
            return storage_;
        }

        Options& Options::setEexcutable(std::function<void(std::uint64_t, const std::string&)> executable)
        {
            executable_ = std::move(executable);
            return *this;
        }

        const std::function<void(std::uint64_t, const std::string&)>& Options::getEexcutable() const
        {
            return executable_;
        }

        Options& Options::setRandomSeed(std::size_t seed)
        {
            randomSeed_ = seed;
            return *this;
        }

        std::size_t Options::getRandomSeed() const
        {
            return randomSeed_;
        }

        Options& Options::setElectionTimeout(std::uint64_t minElectionTimeout, std::uint64_t maxElectionTimeout)
        {
            minElectionTimeout_ = minElectionTimeout;
            maxElectionTimeout_ = maxElectionTimeout;
            return *this;
        }

        std::uint64_t Options::getMinElectionTimeout() const
        {
            return minElectionTimeout_;
        }

        std::uint64_t Options::getMaxElectionTimeout() const
        {
            return maxElectionTimeout_;
        }

        Options& Options::setHeartbeatTimeout(std::uint64_t heartbeatTimeout)
        {
            heartbeatTimeout_ = heartbeatTimeout;
            return *this;
        }

        std::uint64_t Options::getHeatbeatTimeout() const
        {
            return heartbeatTimeout_;
        }

        Options& Options::setMaxWaitEntriesNum(std::uint64_t maxWaitEntriesNum)
        {
            maxWaitEntriesNum_ = maxWaitEntriesNum;
            return *this;
        }

        std::uint64_t Options::getMaxWaitEntriesNum() const
        {
            return maxWaitEntriesNum_;
        }

        Options& Options::setMaxPacketSize(std::uint64_t maxPacketSize)
        {
            maxPacketLength_ = maxPacketSize;
            return *this;
        }

        std::uint64_t Options::getMaxPacketLength() const
        {
            return maxPacketLength_;
        }
    }
}