#include <cr/raft/raft_engine.h>

#include <algorithm>

#include <cr/common/assert.h>
#include <cr/common/throw.h>
#include <cr/raft/candidate.h>
#include <cr/raft/exception.h>
#include <cr/raft/follower.h>
#include <cr/raft/leader.h>
#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {

        RaftEngine::RaftEngine(const Builder& builder)
            : nodeId_(builder.getNodeId()),
            otherNodeIds_(builder.getOtherNodeIds()),
            storage_(builder.getLogStorage()),
            stateMachine_(builder.getStateMachine()),
            currentTerm_(0),
            lastApplied_(0),
            nowTime_(0),
            currentEnumState_(FOLLOWER),
            nextEnumState_(FOLLOWER),
            electionTimeout_(builder.getElectionTimeout()),
            voteTimeout_(builder.getVoteTimeout())
        {
            // 节点有效性判断
            std::sort(otherNodeIds_.begin(), otherNodeIds_.end());
            if (std::unique(otherNodeIds_.begin(), otherNodeIds_.end()) != otherNodeIds_.end())
            {
                CR_THROW(ArgumentException, "Has Repeated Other Node Id");
            }
            if (std::find(otherNodeIds_.begin(), otherNodeIds_.end(), nodeId_) != otherNodeIds_.end())
            {
                CR_THROW(ArgumentException, "Other Node Id List Container This Id");
            }
            if (storage_ == nullptr)
            {
                CR_THROW(ArgumentException, "Log Storage is null");
            }
            if (stateMachine_ == nullptr)
            {
                CR_THROW(ArgumentException, "State Machine is null");
            }
            if (electionTimeout_ == 0)
            {
                CR_THROW(ArgumentException, "Election Timeout is 0");
            }
            if (voteTimeout_.first == 0 || voteTimeout_.second < voteTimeout_.first)
            {
                CR_THROW(ArgumentException, "Vote Timeout Range error");
            }
        }

        RaftEngine::~RaftEngine()
        {}

        std::uint32_t RaftEngine::getNodeId() const
        {
            return nodeId_;
        }

        const std::vector<std::uint32_t>& RaftEngine::getOtherNodeIds() const
        {
            return otherNodeIds_;
        }

        const std::shared_ptr<LogStorage>& RaftEngine::getLogStorage() const
        {
            return storage_;
        }

        const std::shared_ptr<StateMachine>& RaftEngine::getStateMachine() const
        {
            return stateMachine_;
        }

        void RaftEngine::initialize()
        {
            CR_ASSERT(currentState_ == nullptr);
            currentState_ = std::make_shared<Follower>(*this);
            currentState_->onEnter(nullptr);
        }

        std::uint32_t RaftEngine::getCurrentTerm() const
        {
            return currentTerm_;
        }

        boost::optional<std::uint32_t> RaftEngine::getVotedFor() const
        {
            return votedFor_;
        }

        std::uint64_t RaftEngine::getCommitLogIndex() const
        {
            return storage_->getLastIndex();
        }

        std::uint64_t RaftEngine::getLastApplied() const
        {
            return lastApplied_;
        }

        std::uint64_t RaftEngine::getCacheBeginLogIndex() const
        {
            CR_ASSERT(!"Not Implements");
            return 0;
        }

        std::int64_t RaftEngine::getNowTime() const
        {
            return nowTime_;
        }

        RaftEngine::State RaftEngine::getCurrentState() const
        {
            return currentEnumState_;
        }

        const boost::optional<std::uint32_t>& RaftEngine::getLeaderId() const
        {
            return leaderId_;
        }

        std::uint32_t RaftEngine::getElectionTimeout() const
        {
            return electionTimeout_;
        }

        const std::pair<std::uint32_t, std::uint32_t>& RaftEngine::getVoteTimeout() const
        {
            return voteTimeout_;
        }

        void RaftEngine::pushMessage(RaftMsgPtr message)
        {
            CR_ASSERT(message != nullptr)(nodeId_);
            messages_.emplace_back(std::move(message));
        }

        RaftEngine::RaftMsgPtr RaftEngine::popMessage()
        {
            RaftMsgPtr message;
            if (!messages_.empty())
            {
                message = std::move(messages_.front());
                messages_.pop_front();
            }
            return message;
        }

        std::int64_t RaftEngine::update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            CR_ASSERT(currentState_ != nullptr);
            nowTime_ = nowTime;
            std::int64_t nextUpdateTime = currentState_->update(nowTime, outMessages);
            if (nextEnumState_ != currentEnumState_)
            {
                onTransitionState();
                nextUpdateTime = nowTime;
            }
            CR_ASSERT(nextEnumState_ == currentEnumState_);
            return nextUpdateTime;
        }

        void RaftEngine::setNextState(State nextState)
        {
            nextEnumState_ = nextState;
        }

        void RaftEngine::onTransitionState()
        {
            currentState_->onLeave();
            auto prevState = std::move(currentState_);
            switch (nextEnumState_)
            {
            case FOLLOWER:
                currentState_ = std::make_shared<Follower>(*this);
                break;
            case CANDIDATE:
                currentState_ = std::make_shared<Candidate>(*this);
                break;
            case LEADER:
                currentState_ = std::make_shared<Leader>(*this);
                break;
            default:
                CR_ASSERT(!"Exception State")(static_cast<int>(currentEnumState_))(static_cast<int>(nextEnumState_));
                break;
            }
            currentState_->onEnter(std::move(prevState));
            currentEnumState_ = nextEnumState_;
        }

        void RaftEngine::setCurrentTerm(std::uint32_t currentTerm)
        {
            currentTerm_ = currentTerm;
        }

        void RaftEngine::setVotedFor(boost::optional<std::uint32_t> voteFor)
        {
            votedFor_ = voteFor;
        }

        void RaftEngine::setLastApplied(std::uint64_t lastApplied)
        {
            lastApplied_ = lastApplied;
        }

        void RaftEngine::setLeaderId(boost::optional<std::uint32_t> leaderId)
        {
            leaderId_ = leaderId;
        }
    }
}