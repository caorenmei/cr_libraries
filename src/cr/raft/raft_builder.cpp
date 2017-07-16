#include <cr/raft/raft_builder.h>

#include <cr/raft/raft.h>

namespace cr
{
    namespace raft
    {
        RaftBuilder::RaftBuilder()
            : nodeId_(0),
            minElectionTimeout_(150),
            maxElectionTimeout_(200),
            heartbeatTimeout_(50),
            randomSeed_(0),
            maxWaitEntriesNum_(1),
            maxPacketEntriesNum_(1),
            maxPacketLength_(1)
        {}

        RaftBuilder::~RaftBuilder()
        {}

        RaftBuilder& RaftBuilder::setNodeId(std::uint64_t nodeId)
        {
            nodeId_ = nodeId;
            return *this;
        }

        std::uint64_t  RaftBuilder::getNodeId() const
        {
            return nodeId_;
        }

        RaftBuilder& RaftBuilder::setBuddyNodeIds(std::vector<std::uint64_t> otherNodeIds)
        {
            buddyNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        const std::vector<std::uint64_t>& RaftBuilder::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        RaftBuilder& RaftBuilder::setStorage(std::shared_ptr<Storage> storage)
        {
            storage_ = std::move(storage);
            return *this;
        }

        const std::shared_ptr<Storage>& RaftBuilder::getStorage() const
        {
            return storage_;
        }

        RaftBuilder& RaftBuilder::setEexcuteCallback(std::function<void(std::uint64_t, const std::string&)> cb)
        {
            executable_ = std::move(cb);
            return *this;
        }

        const std::function<void(std::uint64_t, const std::string&)>& RaftBuilder::getEexcuteCallback() const
        {
            return executable_;
        }

        RaftBuilder& RaftBuilder::setRandomSeed(std::size_t seed)
        {
            randomSeed_ = seed;
            return *this;
        }

        std::size_t RaftBuilder::getRandomSeed() const
        {
            return randomSeed_;
        }

        RaftBuilder& RaftBuilder::setElectionTimeout(std::uint64_t minElectionTimeout, std::uint64_t maxElectionTimeout)
        {
            minElectionTimeout_ = minElectionTimeout;
            maxElectionTimeout_ = maxElectionTimeout;
            return *this;
        }

        std::uint64_t RaftBuilder::getMinElectionTimeout() const
        {
            return minElectionTimeout_;
        }

        std::uint64_t RaftBuilder::getMaxElectionTimeout() const
        {
            return maxElectionTimeout_;
        }

        RaftBuilder& RaftBuilder::setHeartbeatTimeout(std::uint64_t heartbeatTimeout)
        {
            heartbeatTimeout_ = heartbeatTimeout;
            return *this;
        }

        std::uint64_t RaftBuilder::getHeatbeatTimeout() const
        {
            return heartbeatTimeout_;
        }

        RaftBuilder& RaftBuilder::setMaxWaitEntriesNum(std::uint64_t maxWaitEntriesNum)
        {
            maxWaitEntriesNum_ = maxWaitEntriesNum;
            return *this;
        }

        std::uint64_t RaftBuilder::getMaxWaitEntriesNum() const
        {
            return maxWaitEntriesNum_;
        }

        RaftBuilder& RaftBuilder::setMaxPacketEntriesNum(std::uint64_t maxPacketEntriesNum)
        {
            maxPacketEntriesNum_ = maxPacketEntriesNum;
            return *this;
        }

        std::uint64_t RaftBuilder::getMaxPacketEntriesNum() const
        {
            return maxPacketEntriesNum_;
        }

        RaftBuilder& RaftBuilder::setMaxPacketSize(std::uint64_t maxPacketSize)
        {
            maxPacketLength_ = maxPacketSize;
            return *this;
        }

        std::uint64_t RaftBuilder::getMaxPacketLength() const
        {
            return maxPacketLength_;
        }

        std::shared_ptr<Raft> RaftBuilder::build()
        {
            return std::make_shared<Raft>(*this);
        }
    }
}
