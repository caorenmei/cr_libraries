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
            storage_(builder.getStorage()),
            executeCallback_(builder.getEexcuteCallback()),
            random_(builder.getRandom()),
            logWindowSize_(builder.getLogWindowSize()),
            maxPacketLength_(builder.getMaxPacketLength()),
            currentTerm_(storage_->lastTerm()),
            commitIndex_(0),
            lastApplied_(0),
            nowTime_(0),
            currentEnumState_(FOLLOWER),
            nextEnumState_(FOLLOWER),
            electionTimeout_(builder.getElectionTimeout()),
            heatbeatTimeout_(builder.getHeatbeatTimeout())
        {
            std::sort(buddyNodeIds_.begin(), buddyNodeIds_.end());
            // 节点有效性判断
            CR_ASSERT(std::unique(buddyNodeIds_.begin(), buddyNodeIds_.end()) == buddyNodeIds_.end());
            CR_ASSERT(std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId_) == buddyNodeIds_.end());
            CR_ASSERT(storage_ != nullptr);
            CR_ASSERT(executeCallback_ != nullptr);
            CR_ASSERT(electionTimeout_.first != 0 && electionTimeout_.first <= electionTimeout_.second);
            CR_ASSERT(heatbeatTimeout_ != 0 && heatbeatTimeout_ <= electionTimeout_.first);
            CR_ASSERT(random_ != nullptr);
            CR_ASSERT(logWindowSize_ >= 1);
            CR_ASSERT(maxPacketLength_ >= 1);
        }

        RaftEngine::~RaftEngine()
        {}

        std::uint64_t RaftEngine::getNodeId() const
        {
            return nodeId_;
        }

        const std::vector<std::uint64_t>& RaftEngine::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        bool RaftEngine::isBuddyNodeId(std::uint64_t nodeId) const
        {
            return std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId) != buddyNodeIds_.end();
        }

        const std::shared_ptr<Storage>& RaftEngine::getStorage() const
        {
            return storage_;
        }

        std::uint64_t RaftEngine::getNowTime() const
        {
            return nowTime_;
        }

        void RaftEngine::initialize(std::uint64_t nowTime)
        {
            CR_ASSERT(nowTime >= 0 && currentState_ == nullptr)(nowTime)(currentState_.get());
            nowTime_ = nowTime;
            currentState_ = std::make_shared<Follower>(*this);
            currentState_->onEnter(nullptr);
        }

        std::uint64_t RaftEngine::update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            CR_ASSERT(currentState_ != nullptr);
            if (nowTime < nowTime_)
            {
                return nowTime_;
            }
            nowTime_ = nowTime;

            auto nextUpdateTime = currentState_->update(nowTime, outMessages);
            CR_ASSERT(nextUpdateTime >= nowTime_)(nextUpdateTime)(nowTime_);
            if (nextEnumState_ != currentEnumState_)
            {
                onTransitionState();
                nextUpdateTime = nowTime;
            }
            CR_ASSERT(nextEnumState_ == currentEnumState_);
            CR_ASSERT(nextUpdateTime >= nowTime_)(nextUpdateTime)(nowTime_);

            if (lastApplied_ < commitIndex_)
            {
                ++lastApplied_;
                auto entries = storage_->entries(lastApplied_, lastApplied_);
                for (auto&& entry : entries)
                {
                    executeCallback_(entry.getIndex(), entry.getValue());
                }
            }

            if (!messages_.empty() || lastApplied_ < commitIndex_)
            {
                return nowTime_;
            }
            return nextUpdateTime;
        }

        std::deque<RaftEngine::RaftMsgPtr>& RaftEngine::getMessageQueue()
        {
            return messages_;
        }

        std::uint64_t RaftEngine::getHeatbeatTimeout() const
        {
            return heatbeatTimeout_;
        }

        std::uint64_t RaftEngine::getMinElectionTimeout() const
        {
            return electionTimeout_.first;
        }

        void RaftEngine::execute(std::string value)
        {
            if (currentEnumState_ == LEADER)
            {
                auto lastLogIndex = storage_->lastIndex();
                storage_->append({ lastLogIndex + 1, currentTerm_, std::move(value) });
            }
        }

        std::uint64_t RaftEngine::getCommitIndex() const
        {
            return commitIndex_;
        }

        std::uint64_t RaftEngine::getLastApplied() const
        {
            return lastApplied_;
        }

        RaftEngine::State RaftEngine::getCurrentState() const
        {
            return currentEnumState_;
        }

        std::uint64_t RaftEngine::getCurrentTerm() const
        {
            return currentTerm_;
        }

        boost::optional<std::uint64_t> RaftEngine::getVotedFor() const
        {
            return votedFor_;
        }

        const boost::optional<std::uint64_t>& RaftEngine::getLeaderId() const
        {
            return leaderId_;
        }

        std::uint64_t RaftEngine::getLogWindowSize() const
        {
            return logWindowSize_;
        }

        std::uint64_t RaftEngine::getMaxPacketLength() const
        {
            return maxPacketLength_;
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

        void RaftEngine::setCurrentTerm(std::uint64_t currentTerm)
        {
            CR_ASSERT(currentTerm_ <= currentTerm)(currentTerm_)(currentTerm);
            currentTerm_ = currentTerm;
        }

        void RaftEngine::setVotedFor(boost::optional<std::uint64_t> voteFor)
        {
            votedFor_ = voteFor;
        }

        void RaftEngine::setCommitIndex(std::uint64_t commitIndex)
        {
            CR_ASSERT(commitIndex_ <= commitIndex_)(commitIndex)(commitIndex_);
            commitIndex_ = commitIndex;
        }

        void RaftEngine::setLeaderId(boost::optional<std::uint64_t> leaderId)
        {
            leaderId_ = leaderId;
        }

        std::uint64_t RaftEngine::randomElectionTimeout() const
        {
            auto electionTime = electionTimeout_.first;
            if (electionTimeout_.first < electionTimeout_.second)
            {
                electionTime += random_() % (electionTimeout_.second - electionTimeout_.first);
            } 
            return electionTime;
        }
    }
}