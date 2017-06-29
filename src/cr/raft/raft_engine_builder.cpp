#include <cr/raft/raft_engine_builder.h>

#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        RaftEngineBuilder::RaftEngineBuilder()
            : nodeId_(0),
            electionTimeout_(0)
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

        RaftEngineBuilder& RaftEngineBuilder::setOtherNodeIds(std::vector<std::uint32_t> otherNodeIds)
        {
            buddyNodeIds_ = std::move(otherNodeIds);
            return *this;
        }

        const std::vector<std::uint32_t>& RaftEngineBuilder::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setLogStorage(std::shared_ptr<LogStorage> storage)
        {
            storage_ = std::move(storage);
            return *this;
        }

        const std::shared_ptr<LogStorage>& RaftEngineBuilder::getLogStorage() const
        {
            return storage_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setStateMachine(std::shared_ptr<StateMachine> stateMachine)
        {
            stateMachine_ = std::move(stateMachine);
            return *this;
        }

        const std::shared_ptr<StateMachine>&  RaftEngineBuilder::getStateMachine() const
        {
            return stateMachine_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setElectionTimeout(std::uint32_t electionTimeout)
        {
            electionTimeout_ = electionTimeout;
            return *this;
        }

        std::uint32_t RaftEngineBuilder::getElectionTimeout() const
        {
            return electionTimeout_;
        }

        RaftEngineBuilder& RaftEngineBuilder::setVoteTimeout(const std::pair<std::uint32_t, std::uint32_t>& voteTimeout)
        {
            voteTimeout_ = voteTimeout;
            return *this;
        }

        const std::pair<std::uint32_t, std::uint32_t>& RaftEngineBuilder::getVoteTimeout() const
        {
            return voteTimeout_;
        }

        std::shared_ptr<RaftEngine> RaftEngineBuilder::build()
        {
            return std::make_shared<RaftEngine>(*this);
        }
    }
}
