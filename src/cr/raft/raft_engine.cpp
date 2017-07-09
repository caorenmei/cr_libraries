#include <cr/raft/raft_engine.h>

#include <algorithm>

#include <cr/common/assert.h>
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
            executable_(builder.getEexcuteCallback()),
            logWindowSize_(builder.getLogWindowSize()),
            maxPacketLength_(builder.getMaxPacketLength()),
            currentTerm_(storage_->getLastTerm()),
            commitIndex_(0),
            lastApplied_(0),
            random_(builder.getRandomSeed()),
            minElectionTimeout_(builder.getMinElectionTimeout()),
            maxElectionTimeout_(builder.getMaxElectionTimeout()),
            heatbeatTimeout_(builder.getHeatbeatTimeout()),
            nowTime_(0),
            nextState_(FOLLOWER)
        {
            std::sort(buddyNodeIds_.begin(), buddyNodeIds_.end());
            // 伙伴节点不能重复
            CR_ASSERT_E(ArgumentException, std::unique(buddyNodeIds_.begin(), buddyNodeIds_.end()) == buddyNodeIds_.end());
            // 伙伴节点Id不能为自身Id
            CR_ASSERT_E(ArgumentException, std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId_) == buddyNodeIds_.end());
            // 日志存储不能为null
            CR_ASSERT_E(ArgumentException, storage_ != nullptr);
            // 复制状态机不能为null
            CR_ASSERT_E(ArgumentException, executable_ != nullptr);
            // 最小选举超时时间不能大于最大选举超时时间
            CR_ASSERT_E(ArgumentException, minElectionTimeout_ != 0 && minElectionTimeout_ <= maxElectionTimeout_)(minElectionTimeout_)(maxElectionTimeout_);
            // 心跳不能大于选举超时时间
            CR_ASSERT_E(ArgumentException, heatbeatTimeout_ != 0 && heatbeatTimeout_ <= minElectionTimeout_)(heatbeatTimeout_)(minElectionTimeout_);
            // 日志复制窗口不能0
            CR_ASSERT_E(ArgumentException, logWindowSize_ >= 1)(logWindowSize_);
            // 最大数据大小不能为0
            CR_ASSERT_E(ArgumentException, maxPacketLength_ >= 1)(maxPacketLength_);
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
            if (nextState_ != currentState_->getState())
            {
                onTransitionState();
                nextUpdateTime = nowTime;
            }
            CR_ASSERT(nextState_ == currentState_->getState());
            CR_ASSERT(nextUpdateTime >= nowTime_)(nextUpdateTime)(nowTime_);

            if (lastApplied_ < commitIndex_)
            {
                ++lastApplied_;
                auto getEntries = storage_->getEntries(lastApplied_, lastApplied_);
                for (auto&& entry : getEntries)
                {
                    executable_(entry.getIndex(), entry.getValue());
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
            return minElectionTimeout_;
        }

        bool RaftEngine::execute(const std::vector<std::string>& values)
        {
            if (currentState_->getState() == LEADER)
            {
                std::vector<Entry> entries;
                auto logIndex = storage_->getLastIndex();
                for (auto&& value : values)
                {
                    logIndex = logIndex + 1;
                    entries.emplace_back(logIndex, currentTerm_, value);
                }
                storage_->append(entries);
                return true;
            }
            return false;
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
            return static_cast<State>(currentState_->getState());
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
            nextState_ = nextState;
        }

        void RaftEngine::onTransitionState()
        {
            currentState_->onLeave();
            auto prevState = std::move(currentState_);
            switch (nextState_)
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
                CR_ASSERT(!"Exception State")(static_cast<int>(currentState_->getState()))(static_cast<int>(nextState_));
                break;
            }
            currentState_->onEnter(std::move(prevState));
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

        std::uint64_t RaftEngine::randomElectionTimeout()
        {
            std::uniform_int_distribution<std::uint64_t> distribution(minElectionTimeout_, maxElectionTimeout_);
            auto electionTime = distribution(random_);
            return electionTime;
        }
    }
}