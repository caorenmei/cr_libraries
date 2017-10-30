#include "raft.h"

#include <algorithm>

#include <cr/core/assert.h>

#include "candidate.h"
#include "exception.h"
#include "follower.h"
#include "leader.h"
#include "raft_msg.pb.h"
#include "raft_state.h"

namespace cr
{
    namespace raft
    {

        Raft::Raft(const Builder& builder)
            : random_(builder.getRandomSeed()),
            minElectionTimeout_(builder.getMinElectionTimeout()),
            maxElectionTimeout_(builder.getMaxElectionTimeout()),
            heatbeatTimeout_(builder.getHeatbeatTimeout()),
            maxWaitEntriesNum_(builder.getMaxWaitEntriesNum()),
            maxPacketEntriesNum_(builder.getMaxPacketEntriesNum()),
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
            CR_ASSERT_E(ArgumentException, maxPacketEntriesNum_ >= 1)(maxPacketEntriesNum_);
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

        Raft::~Raft()
        {}

        std::uint64_t Raft::getMinElectionTimeout() const
        {
            return minElectionTimeout_;
        }

        std::uint64_t Raft::getMaxElectionTimeout() const
        {
            return maxElectionTimeout_;
        }

        std::uint64_t Raft::getHeatbeatTimeout() const
        {
            return heatbeatTimeout_;
        }

        std::uint64_t Raft::getMaxWaitEntriesNum() const
        {
            return maxWaitEntriesNum_;
        }

        std::uint64_t Raft::getMaxPacketEntriesNum() const
        {
            return maxPacketEntriesNum_;
        }

        std::uint64_t Raft::getMaxPacketLength() const
        {
            return maxPacketLength_;
        }

        std::uint64_t Raft::getNowTime() const
        {
            return nowTime_;
        }

        Raft::State Raft::getCurrentState() const
        {
            return static_cast<State>(currentState_->getState());
        }

        void Raft::initialize(std::uint64_t nowTime)
        {
            CR_ASSERT(nowTime >= 0 && currentState_ == nullptr)(nowTime)(currentState_.get());
            nowTime_ = nowTime;
            currentState_ = std::make_shared<Follower>(*this);
            currentState_->onEnter(nullptr);
        }

        std::uint64_t Raft::update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
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

        std::deque<Raft::RaftMsgPtr>& Raft::getMessageQueue()
        {
            return messages_;
        }

        void Raft::pushMessageQueue(RaftMsgPtr raftMsg)
        {
            CR_ASSERT_E(ArgumentException, raftMsg != nullptr);
            CR_ASSERT_E(ArgumentException, raftMsg->dest_node_id() == nodeId_)(raftMsg->dest_node_id())(nodeId_);
            CR_ASSERT_E(ArgumentException, isBuddyNodeId(raftMsg->from_node_id()))(raftMsg->from_node_id());
            messages_.push_back(std::move(raftMsg));
        }

        void Raft::execute(const std::vector<std::string>& values)
        {
            CR_ASSERT_E(StateException, currentState_->getState() == LEADER)(currentState_->getState())(LEADER);
            std::vector<pb::Entry> entries;
            auto logIndex = storage_->getLastIndex();
            for (auto&& value : values)
            {
                entries.emplace_back();
                entries.back().set_term(currentTerm_);
                entries.back().set_value(std::move(value));
            }
            storage_->append(logIndex + 1, entries);
        }

        std::uint64_t Raft::getNodeId() const
        {
            return nodeId_;
        }

        const std::vector<std::uint64_t>& Raft::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        bool Raft::isBuddyNodeId(std::uint64_t nodeId) const
        {
            return std::find(buddyNodeIds_.begin(), buddyNodeIds_.end(), nodeId) != buddyNodeIds_.end();
        }

        boost::optional<std::uint64_t> Raft::getVotedFor() const
        {
            return votedFor_;
        }

        boost::optional<std::uint64_t> Raft::getLeaderId() const
        {
            return leaderId_;
        }

        std::uint64_t Raft::getCurrentTerm() const
        {
            return currentTerm_;
        }

        std::uint64_t Raft::getCommitIndex() const
        {
            return commitIndex_;
        }

        std::uint64_t Raft::getLastApplied() const
        {
            return lastApplied_;
        }

        std::uint64_t Raft::randElectionTimeout()
        {
            std::uniform_int_distribution<std::uint64_t> distribution(minElectionTimeout_, maxElectionTimeout_);
            auto electionTime = distribution(random_);
            return electionTime;
        }

        const std::shared_ptr<Storage>& Raft::getStorage() const
        {
            return storage_;
        }

        void Raft::setNextState(State nextState)
        {
            nextState_ = nextState;
        }

        void Raft::onTransitionState()
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

        void Raft::applyStateMachine()
        {
            auto nextApplied = std::min(lastApplied_ + 10, commitIndex_);
            auto logIndex = lastApplied_ + 1;
            auto entries = storage_->getEntries(logIndex, nextApplied, std::numeric_limits<std::uint64_t>::max());
            for (auto&& entry : entries)
            {
                executable_(logIndex, entry.value());
                logIndex = logIndex + 1;
            }
            lastApplied_ = lastApplied_ + entries.size();
        }

        void Raft::setVotedFor(boost::optional<std::uint64_t> voteFor)
        {
            votedFor_ = voteFor;
        }

        void Raft::setCommitIndex(std::uint64_t commitIndex)
        {
            CR_ASSERT(commitIndex_ <= commitIndex_)(commitIndex)(commitIndex_);
            commitIndex_ = commitIndex;
        }

        void Raft::setLeaderId(boost::optional<std::uint64_t> leaderId)
        {
            leaderId_ = leaderId;
        }

        void Raft::setCurrentTerm(std::uint64_t currentTerm)
        {
            CR_ASSERT(currentTerm_ <= currentTerm)(currentTerm_)(currentTerm);
            currentTerm_ = currentTerm;
        }
    }
}