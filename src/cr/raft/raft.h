#ifndef CR_RAFT_RAFT_H_
#define CR_RAFT_RAFT_H_

#include <cstdint>
#include <deque>
#include <memory>
#include <random>

#include <boost/optional.hpp>

#include <cr/raft/storage.h>
#include <cr/raft/raft_builder.h>

namespace cr
{
    namespace raft
    {

        namespace pb
        {
            class RaftMsg;
        }

        class RaftState;

        template <typename T>
        class DebugVisitor;

        // Raft 引擎
        class Raft
        {
        public:

            using RaftMsgPtr = std::shared_ptr<pb::RaftMsg>;

            using Builder = RaftBuilder;

            // 状态
            enum State : int
            {
                // 跟随者
                FOLLOWER,
                // 候选者
                CANDIDATE,
                // 领导者
                LEADER,
            };

            explicit Raft(const Builder& builder);

            ~Raft();

            Raft(const Raft&) = delete;
            Raft& operator=(const Raft&) = delete;

            // 最小超时时间
            std::uint64_t getMinElectionTimeout() const;

            // 最大超时时间
            std::uint64_t getMaxElectionTimeout() const;

            // 最大超时时间
            std::uint64_t getHeatbeatTimeout() const;

            // 最大传输日志数
            std::uint64_t getMaxWaitEntriesNum() const;

            // 一次最大传输日志数
            std::uint64_t getMaxPacketEntriesNum() const;

            // 最大传输包大小
            std::uint64_t getMaxPacketLength() const;

            // 当前时间
            std::uint64_t getNowTime() const;

            // 当前状态
            State getCurrentState() const;

            // 初始化
            void initialize(std::uint64_t nowTime);

            // 执行逻辑
            std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 添加消息到消息队列
            void pushMessageQueue(RaftMsgPtr raftMsg);

            // 执行一个日志
            void execute(const std::vector<std::string>& values);

            // 自身Id
            std::uint64_t getNodeId() const;

            // 其它节点Id
            const std::vector<std::uint64_t>& getBuddyNodeIds() const;

            // 是否是伙伴Id
            bool isBuddyNodeId(std::uint64_t nodeId) const;

            // 当前投票
            boost::optional<std::uint64_t> getVotedFor() const;

            // 当前领导者
            boost::optional<std::uint64_t> getLeaderId() const;

            // 当前任期
            std::uint64_t getCurrentTerm() const;

            // 最大已提交日志索引
            std::uint64_t getCommitIndex() const;

            // 最后应用到状态机的日志索引
            std::uint64_t getLastApplied() const;

        private:

            template <typename T>
            friend class DebugVisitor;
         
            friend class Follower;
            friend class Candidate;
            friend class Leader;

            // 随机超时时间
            std::uint64_t randElectionTimeout();

            // 底层存储
            const std::shared_ptr<Storage>& getStorage() const;
       
            // 设置下一个状态
            void setNextState(State nextState);

            // 状态转换
            void onTransitionState();

            // 应用状态机
            void applyStateMachine();

            // 输入消息队列
            std::deque<RaftMsgPtr>& getMessageQueue();

            // 设置选票候选者
            void setVotedFor(boost::optional<std::uint64_t> voteFor);

            // 设置领导者
            void setLeaderId(boost::optional<std::uint64_t> leaderId);

            // 设置当前任期
            void setCurrentTerm(std::uint64_t currentTerm);

            // 设置提交日志索引
            void setCommitIndex(std::uint64_t commitIndex);

            std::default_random_engine random_;
            std::uint64_t minElectionTimeout_;
            std::uint64_t maxElectionTimeout_;
            std::uint64_t heatbeatTimeout_; 
            std::uint64_t maxWaitEntriesNum_;
            std::uint64_t maxPacketEntriesNum_;
            std::uint64_t maxPacketLength_;
            
            std::shared_ptr<Storage> storage_;
            std::function<void(std::uint64_t, const std::string&)> executable_;

            std::uint64_t nowTime_;
            std::deque<RaftMsgPtr> messages_;
            std::shared_ptr<RaftState> currentState_;
            State nextState_;

            std::uint64_t nodeId_;
            std::vector<std::uint64_t> buddyNodeIds_;
            boost::optional<std::uint64_t> votedFor_;
            boost::optional<std::uint64_t> leaderId_;

            std::uint64_t currentTerm_;
            std::uint64_t commitIndex_;
            std::uint64_t lastApplied_;
        };
    }
}

#endif