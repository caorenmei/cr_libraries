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
            : random_(builder.getRandomSeed()),
            minElectionTimeout_(builder.getMinElectionTimeout()),
            maxElectionTimeout_(builder.getMaxElectionTimeout()),
            heatbeatTimeout_(builder.getHeatbeatTimeout()),
            maxEntriesNum_(builder.getMaxEntriesNum()),
            maxPacketLength_(builder.getMaxPacketLength()), 
            storage_(builder.getStorage()),
            executable_(builder.getEexcuteCallback()),
            nowTime_(0),
            nextState_(FOLLOWER),
            nodeId_(builder.getNodeId()),
            buddyNodeIds_(builder.getBuddyNodeIds()),
            currentTerm_(storage_->getLastTerm()),
            commitIndex_(0),
            lastApplied_(0)
        {
            std::sort(buddyNodeIds_.begin(), buddyNodeIds_.end());
            // 最小选举超时时间不能大于最大选举超时时间
            CR_ASSERT_E(ArgumentException, minElectionTimeout_ != 0 && minElectionTimeout_ <= maxElectionTimeout_)(minElectionTimeout_)(maxElectionTimeout_);
            // 心跳不能大于选举超时时间
            CR_ASSERT_E(ArgumentException, heatbeatTimeout_ != 0 && heatbeatTimeout_ <= minElectionTimeout_)(heatbeatTimeout_)(minElectionTimeout_);
            // 日志复制窗口不能0
            CR_ASSERT_E(ArgumentException, maxEntriesNum_ >= 1)(maxEntriesNum_);
            // 最大数据大小不能为0
            CR_ASSERT_E(ArgumentException, maxPacketLength_ >= 1)(maxPacketLength_);
            // 日志存储不能为null
            CR_ASSERT_E(ArgumentException, storage_ != nullptr);
            // 复制状态机不能为null
            CR_ASSERT_E(ArgumentException, executable_ != nullptr);
            // 伙伴节点不能重复
            CR_ASSERT_E(ArgumentException, std::unique(buddyNodeIds_.begin(), buddyNodeIds_.end()) == buddyNodeIds_.end());
            // 伙伴节点Id不能为自身Id
            CR_ASSERT_E(ArgumentException, std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId_) == buddyNodeIds_.end());   
        }

        RaftEngine::~RaftEngine()
        {}

        std::uint64_t RaftEngine::getMinElectionTimeout() const
        {
            return minElectionTimeout_;
        }

        std::uint64_t RaftEngine::getMaxElectionTimeout() const
        {
            return maxElectionTimeout_;
        }

        std::uint64_t RaftEngine::getHeatbeatTimeout() const
        {
            return heatbeatTimeout_;
        }

        std::uint64_t RaftEngine::getMaxEntriesNum() const
        {
            return maxEntriesNum_;
        }

        std::uint64_t RaftEngine::getMaxPacketLength() const
        {
            return maxPacketLength_;
        }

        std::uint64_t RaftEngine::getNowTime() const
        {
            return nowTime_;
        }

        RaftEngine::State RaftEngine::getCurrentState() const
        {
            return static_cast<State>(currentState_->getState());
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
            // 更新时间
            CR_ASSERT(nowTime_ <= nowTime)(nowTime_)(nowTime);
            nowTime_ = nowTime;
            // 处理Raft状态
            auto nextUpdateTime = currentState_->update(nowTime, outMessages);
            CR_ASSERT(nextUpdateTime >= nowTime_)(nextUpdateTime)(nowTime_);
            // 状态转换
            if (nextState_ != currentState_->getState())
            {
                onTransitionState();
                CR_ASSERT(nextState_ == currentState_->getState())(nextState_)(currentState_->getState());
                nextUpdateTime = nowTime;
            }
            // 引用状态机
            if (lastApplied_ < commitIndex_)
            {
                applyStateMachine();
                CR_ASSERT(lastApplied_ <= commitIndex_)(lastApplied_)(commitIndex_);
            }
            // 继续循环处理
            if (!messages_.empty() || lastApplied_ < commitIndex_)
            {
                nextUpdateTime = nowTime;
            }
            return nextUpdateTime;
        }

        std::deque<RaftEngine::RaftMsgPtr>& RaftEngine::getMessageQueue()
        {
            return messages_;
        }

        void RaftEngine::pushMessageQueue(RaftMsgPtr raftMsg)
        {
            messages_.push_back(std::move(raftMsg));
        }

        void RaftEngine::execute(const std::vector<std::string>& values)
        {
            CR_ASSERT_E(StateException, currentState_->getState() == LEADER)(currentState_->getState())(LEADER);
            std::vector<Entry> entries;
            auto logIndex = storage_->getLastIndex();
            for (auto&& value : values)
            {
                logIndex = logIndex + 1;
                entries.emplace_back(logIndex, currentTerm_, value);
            }
            storage_->append(entries);
        }

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

        boost::optional<std::uint64_t> RaftEngine::getVotedFor() const
        {
            return votedFor_;
        }

        boost::optional<std::uint64_t> RaftEngine::getLeaderId() const
        {
            return leaderId_;
        }

        std::uint64_t RaftEngine::getCurrentTerm() const
        {
            return currentTerm_;
        }

        std::uint64_t RaftEngine::getCommitIndex() const
        {
            return commitIndex_;
        }

        std::uint64_t RaftEngine::getLastApplied() const
        {
            return lastApplied_;
        }

        std::uint64_t RaftEngine::randElectionTimeout()
        {
            std::uniform_int_distribution<std::uint64_t> distribution(minElectionTimeout_, maxElectionTimeout_);
            auto electionTime = distribution(random_);
            return electionTime;
        }

        const std::shared_ptr<Storage>& RaftEngine::getStorage() const
        {
            return storage_;
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

        void RaftEngine::applyStateMachine()
        {
            auto nextApplied = std::min(lastApplied_ + 10, commitIndex_);
            auto getEntries = storage_->getEntries(lastApplied_ + 1, nextApplied);
            for (auto&& entry : getEntries)
            {
                executable_(entry.getIndex(), entry.getValue());
            }
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

        void RaftEngine::setCurrentTerm(std::uint64_t currentTerm)
        {
            CR_ASSERT(currentTerm_ <= currentTerm)(currentTerm_)(currentTerm);
            currentTerm_ = currentTerm;
        }
    }
}