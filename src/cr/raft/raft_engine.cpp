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
            buddyNodeIds_(builder.getBuddyNodeIds()),
            storage_(builder.getLogStorage()),
            stateMachine_(builder.getStateMachine()),
            currentTerm_(0),
            commitIndex_(0),
            lastApplied_(0),
            nowTime_(0),
            currentEnumState_(FOLLOWER),
            nextEnumState_(FOLLOWER),
            electionTimeout_(builder.getElectionTimeout()),
            voteTimeout_(builder.getVoteTimeout())
        {
            // 节点有效性判断
            std::sort(buddyNodeIds_.begin(), buddyNodeIds_.end());
            if (std::unique(buddyNodeIds_.begin(), buddyNodeIds_.end()) != buddyNodeIds_.end())
            {
                CR_THROW(ArgumentException, "Has Repeated Other Node Id");
            }
            if (std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId_) != buddyNodeIds_.end())
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

        const std::vector<std::uint32_t>& RaftEngine::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        bool RaftEngine::isBuddyNodeId(std::uint32_t nodeId) const
        {
            return std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId) != buddyNodeIds_.end();
        }

        const std::shared_ptr<LogStorage>& RaftEngine::getLogStorage() const
        {
            return storage_;
        }

        const std::shared_ptr<StateMachine>& RaftEngine::getStateMachine() const
        {
            return stateMachine_;
        }

        void RaftEngine::initialize(std::int64_t nowTime)
        {
            CR_ASSERT(nowTime >= 0 && currentState_ == nullptr)(nowTime)(currentState_.get());
            nowTime_ = nowTime;
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

        std::uint64_t RaftEngine::getLastLogIndex() const
        {
            return storage_->getLastIndex();
        }

        std::uint32_t RaftEngine::getLogTerm(std::uint64_t logIndex) const
        {
            if (logIndex != 0)
            {
                return storage_->getTermByIndex(logIndex);
            }
            return 0;
        }

        std::uint64_t RaftEngine::getCommitLogIndex() const
        {
            return commitIndex_;
        }

        std::uint64_t RaftEngine::getLastApplied() const
        {
            return lastApplied_;
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

        void RaftEngine::pushTailMessage(RaftMsgPtr message)
        {
            CR_ASSERT(message != nullptr)(nodeId_);
            messages_.push_back(std::move(message));
        }

        void RaftEngine::pushFrontMessage(RaftMsgPtr message)
        {
            CR_ASSERT(message != nullptr)(nodeId_);
            messages_.push_front(std::move(message));
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
            if (nowTime < nowTime_)
            {
                return nowTime_;
            }
            nowTime_ = nowTime;
            // 更新状态机
            std::int64_t nextUpdateTime = currentState_->update(nowTime, outMessages);
            CR_ASSERT(nextUpdateTime >= nowTime_)(nextUpdateTime)(nowTime_);
            if (nextEnumState_ != currentEnumState_)
            {
                onTransitionState();
                nextUpdateTime = nowTime;
            }
            CR_ASSERT(nextEnumState_ == currentEnumState_);
            CR_ASSERT(nextUpdateTime >= nowTime_)(nextUpdateTime)(nowTime_);
            // 应用日志
            if (lastApplied_ < commitIndex_)
            {
                ++lastApplied_;
                LogEntry logEntry;
                storage_->get(lastApplied_, logEntry);
                stateMachine_->execute(logEntry.getIndex(), logEntry.getValue(), boost::any());
            }
            // 有消息没处理完，立即update
            if (!messages_.empty() || lastApplied_ < commitIndex_)
            {
                return nowTime_;
            }
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

        void RaftEngine::setCommitIndex(std::uint64_t commitIndex)
        {
            commitIndex_ = commitIndex;
        }

        void RaftEngine::appendLog(std::uint64_t logIndex, const std::string& value)
        {
            if (logIndex <= storage_->getLastIndex())
            {
                std::uint32_t logTerm = storage_->getTermByIndex(logIndex);
                if (logTerm != currentTerm_)
                {
                    storage_->del(logIndex);
                }
            }
            storage_->append({ logIndex, currentTerm_, value });
        }

        void RaftEngine::setLeaderId(boost::optional<std::uint32_t> leaderId)
        {
            leaderId_ = leaderId;
        }
    }
}