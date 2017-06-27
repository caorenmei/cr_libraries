#include <cr/raft/raft_engine.h>

#include <algorithm>

#include <cr/common/throw.h>
#include <cr/raft/exception.h>
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
            commitLogIndex_(0),
            lastApplied_(0),
            nowTime_(0)
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
            return commitLogIndex_;
        }

        std::uint64_t RaftEngine::getLastApplied() const
        {
            return lastApplied_;
        }

        std::int64_t RaftEngine::getNowTime() const
        {
            return nowTime_;
        }

        const std::shared_ptr<RaftState>& RaftEngine::getCurrentState() const
        {
            return currentState_;
        }

        void RaftEngine::setNextState(std::shared_ptr<RaftState> nextState)
        {
            nextState_ = std::move(nextState);
        }

        std::int64_t RaftEngine::update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            nowTime_ = nowTime;
            std::int64_t nextUpdateTime = currentState_->update(nowTime, outMessages);
            if (nextState_ != nullptr)
            {
                onTransitionState();
                nextUpdateTime = nowTime;
            }
            return nextUpdateTime;
        }

        std::int64_t RaftEngine::update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages)
        {
            nowTime_ = nowTime;
            std::int64_t nextUpdateTime = currentState_->update(nowTime, std::move(inMessage), outMessages);
            if (nextState_ != nullptr)
            {
                onTransitionState();
                nextUpdateTime = nowTime;
            }
            return nextUpdateTime;
        }

        void RaftEngine::setCurrentTerm(std::uint32_t currentTerm)
        {
            currentTerm_ = currentTerm;
        }

        void RaftEngine::setVotedFor(boost::optional<std::uint32_t> voteFor)
        {
            votedFor_ = voteFor;
        }

        void RaftEngine::setCommitLogIndex(std::uint64_t commitIndex)
        {
            commitLogIndex_ = commitIndex;
        }

        void RaftEngine::setLastApplied(std::uint64_t lastApplied)
        {
            lastApplied_ = lastApplied;
        }

        void RaftEngine::onTransitionState()
        {
            currentState_->onLeave();
            auto prevState = std::move(currentState_);
            currentState_ = std::move(nextState_);
            currentState_->onEnter(prevState);
        }
    }
}