#include <cr/raft/raft_engine_builder.h>

#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        RaftEngineBuilder::RaftEngineBuilder()
            : nodeId_(0),
            minElectionTimeout_(150),
            maxElectionTimeout_(200),
            heartbeatTimeout_(50),
            randomSeed_(0),
            maxEntriesNum_(1),
            maxPacketLength_(1)
        {}

        RaftEngineBuilder::~RaftEngineBuilder()
        {}

        RaftEngineBuilder& RaftEngineBuilder::setNodeId(std::uint64_t nodeId)
        {
            nodeId_ = nodeId;
            return *this;
        }

        std::uint64_t  RaftEngineBuilder::getNodeId() const
        {
            return nodeId_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setBuddyNodeIds(std::vector<std::uint64_t> otherNodeIds)
        {
            buddyNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        const std::vector<std::uint64_t>& RaftEngineBuilder::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setStorage(std::shared_ptr<Storage> storage)
        {
            storage_ = std::move(storage);
            return *this;
        }

        const std::shared_ptr<Storage>& RaftEngineBuilder::getStorage() const
        {
            return storage_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setEexcuteCallback(std::function<void(std::uint64_t, const std::string&)> cb)
        {
            executable_ = std::move(cb);
            return *this;
        }

        const std::function<void(std::uint64_t, const std::string&)>& RaftEngineBuilder::getEexcuteCallback() const
        {
            return executable_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setElectionTimeout(std::uint64_t minElectionTimeout, std::uint64_t maxElectionTimeout)
        {
            minElectionTimeout_ = minElectionTimeout;
            maxElectionTimeout_ = maxElectionTimeout;
            return *this;
        }

        std::uint64_t RaftEngineBuilder::getMinElectionTimeout() const
        {
            return minElectionTimeout_;
        }

        std::uint64_t RaftEngineBuilder::getMaxElectionTimeout() const
        {
            return maxElectionTimeout_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setHeartbeatTimeout(std::uint64_t heartbeatTimeout)
        {
            heartbeatTimeout_ = heartbeatTimeout;
            return *this;
        }

        std::uint64_t RaftEngineBuilder::getHeatbeatTimeout() const
        {
            return heartbeatTimeout_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setRandomSeed(std::size_t seed)
        {
            randomSeed_ = seed;
            return *this;
        }

        std::size_t RaftEngineBuilder::getRandomSeed() const
        {
            return randomSeed_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setMaxEntriesNum(std::uint64_t maxEntriesNum)
        {
            maxEntriesNum_ = maxEntriesNum;
            return *this;
        }

        std::uint64_t RaftEngineBuilder::getMaxEntriesNum() const
        {
            return maxEntriesNum_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setMaxPacketSize(std::uint64_t maxPacketSize)
        {
            maxPacketLength_ = maxPacketSize;
            return *this;
        }

        std::uint64_t RaftEngineBuilder::getMaxPacketLength() const
        {
            return maxPacketLength_;
        }

        std::shared_ptr<RaftEngine> RaftEngineBuilder::build()
        {
            return std::make_shared<RaftEngine>(*this);
        }
    }
}
