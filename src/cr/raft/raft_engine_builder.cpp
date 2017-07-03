#include <cr/raft/raft_engine_builder.h>

#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        RaftEngineBuilder::RaftEngineBuilder()
            : nodeId_(0),
            logWindowSize_(1),
            maxPacketLength_(1)
        {}

        RaftEngineBuilder::~RaftEngineBuilder()
        {}

        RaftEngineBuilder& RaftEngineBuilder::setNodeId(std::uint32_t nodeId)
        {
            nodeId_ = nodeId;
            return *this;
        }

        std::uint32_t  RaftEngineBuilder::getNodeId() const
        {
            return nodeId_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setBuddyNodeIds(std::vector<std::uint32_t> otherNodeIds)
        {
            buddyNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        const std::vector<std::uint32_t>& RaftEngineBuilder::getBuddyNodeIds() const
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
            executeCallback_ = std::move(cb);
            return *this;
        }

        const std::function<void(std::uint64_t, const std::string&)>& RaftEngineBuilder::getEexcuteCallback() const
        {
            return executeCallback_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setElectionTimeout(const std::pair<std::uint32_t, std::uint32_t>& electionTimeout)
        {
            electionTimeout_ = electionTimeout;
            return *this;
        }

        const std::pair<std::uint32_t, std::uint32_t>& RaftEngineBuilder::getElectionTimeout() const
        {
            return electionTimeout_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setRandom(std::function<std::uint32_t()> random)
        {
            random_ = std::move(random);
        }

        const std::function<std::uint32_t()>& RaftEngineBuilder::getRandom() const
        {
            return random_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setLogWindowSize(std::uint32_t logWindowSize)
        {
            logWindowSize_ = logWindowSize;
        }

        std::uint32_t RaftEngineBuilder::getLogWindowSize() const
        {
            return logWindowSize_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setMaxPacketSize(std::uint32_t maxPacketSize)
        {
            maxPacketLength_ = maxPacketSize;
        }

        std::uint32_t RaftEngineBuilder::getMaxPacketLength() const
        {
            return maxPacketLength_;
        }

        std::shared_ptr<RaftEngine> RaftEngineBuilder::build()
        {
            return std::make_shared<RaftEngine>(*this);
        }
    }
}
