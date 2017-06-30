#ifndef CR_RAFT_RAFT_ENGINE_H_
#define CR_RAFT_RAFT_ENGINE_H_

#include <cstdint>
#include <deque>
#include <memory>

#include <boost/optional.hpp>

#include <cr/raft/log_storage.h>
#include <cr/raft/raft_engine_builder.h>
#include <cr/raft/state_machine.h>

namespace cr
{
    namespace raft
    {

        namespace pb
        {
            /** raft 通信协议 */
            class RaftMsg;
        }

        /** Raft状态机 */
        class RaftState;

        /** 调试访问器，可以访问类的私有成员 */
        template <typename T>
        class DebugVisitor;

        /**
         * Raft算法引擎
         * 一个Raft算法引擎对应一个raft实例
         */
        class RaftEngine
        {
        public:

            /** raft消息包 */
            using RaftMsgPtr = std::shared_ptr<pb::RaftMsg>;

            /** Raft引擎构造器 */
            using Builder = RaftEngineBuilder;

            /** 当前状态 */
            enum State
            {
                /** 跟随者状态 */
                FOLLOWER,
                /** 候选者状态 */
                CANDIDATE,
                /** 领导者状态 */
                LEADER,
            };

            /**
             * 构造函数
             * @param builder 构造器
             */
            explicit RaftEngine(const Builder& builder);

            /** 析构函数 */
            ~RaftEngine();

            RaftEngine(const RaftEngine&) = delete;
            RaftEngine& operator=(const RaftEngine&) = delete;

            /**
             * 获取节点Id
             * @return 节点Id
             */
            std::uint32_t getNodeId() const;

            /**
             * 获取其它节点Id
             * @return 其它节点Id
             */
            const std::vector<std::uint32_t>& getBuddyNodeIds() const;

            /**
             * 判断是不是伙伴Id
             * @return True 是, False 其它
             */
            bool isBuddyNodeId(std::uint32_t nodeId) const;

            /**
             * 获取日志存储接口
             * @return 日志存储接口
             */
            const std::shared_ptr<LogStorage>& getLogStorage() const;

            /**
             * 获取状态机接口
             * @return 状态机接口
             */
            const std::shared_ptr<StateMachine>& getStateMachine() const;

            /**
             * 初始化
             * @param nowTime 当前时间
             */
            void initialize(std::int64_t nowTime);

            /**
             * 服务器当前的任期号
             * @param 当前的任期号
             */
            std::uint32_t getCurrentTerm() const;

            /**
             * 获取当前选票的候选人Id
             * @return 当前选票的候选人Id
             */
            boost::optional<std::uint32_t> getVotedFor() const;

            /**
             * 获取已知的最大的已经被提交的日志条目的索引值
             * @return 已知的最大的已经被提交的日志条目的索引值
             */
            std::uint64_t getCommitLogIndex() const;

            /**
             * 获取最后被应用到状态机的日志条目索引值（初始化为 0，持续递增）
             * @return 应用到状态机的日志条目索引值
             */
            std::uint64_t getLastApplied() const;

            /**
             * 获取缓存的日志起始索引
             * @return 缓存的日志起始索引
             */
            std::uint64_t getCacheBeginLogIndex() const;

            /**
             * 获取当前时间戳
             * @param 当前时间戳， 以毫秒计算
             */
            std::int64_t getNowTime() const;

            /*
             * 获取当前状态
             * @param 当前状态
             */
            State getCurrentState() const;

            /**
             * 获取当前的领导者Id
             * @return 领导者Id
             */
            const boost::optional<std::uint32_t>& getLeaderId() const;

            /**
             * 获取选举超时时间
             * @return 选举超时时间，毫秒
             */
            std::uint32_t getElectionTimeout() const;

            /**
             * 获取投票超时时间
             * @return 投票超时时间，毫秒
             */
            const std::pair<std::uint32_t, std::uint32_t>& getVoteTimeout() const;

            /**
             * 加入消息到队列，等待后续处理
             * @param message rpc消息
             */
            void pushTailMessage(RaftMsgPtr message);

            /**
             * 加入消息到队列头部，等待后续处理
             * @param message rpc消息
             */
            void pushFrontMessage(RaftMsgPtr message);

            /**
             * 从队列弹出消息
             * @return rpc消息
             */
            RaftMsgPtr popMessage();

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

        private:

            // 调试模式
            template <typename T>
            friend class DebugVisitor;
        
            // 状态机为友元类 
            friend class Follower;
            friend class Candidate;
            friend class Leader;

            // 设置下一个状态
            void setNextState(State nextState);

            // 状态切换
            void onTransitionState();

            // 设置当前任期编号
            void setCurrentTerm(std::uint32_t currentTerm);

            // 设置当前选票的候选人Id
            void setVotedFor(boost::optional<std::uint32_t> voteFor);

            // 设置获取最后被应用到状态机的日志条目索引值（初始化为 0，持续递增）
            void setLastApplied(std::uint64_t lastApplied);

            // 设置领导者Id
            void setLeaderId(boost::optional<std::uint32_t> leaderId);

            // 节点相关数据

            // 本节点ID
            std::uint32_t nodeId_;
            // 其它节点ID
            std::vector<std::uint32_t> buddyNodeIds_;
            // 存储接口
            std::shared_ptr<LogStorage> storage_;
            // 状态机
            std::shared_ptr<StateMachine> stateMachine_;
            // 消息队列
            std::deque<RaftMsgPtr> messages_;

            // 所有服务器上持久存在的

            // 服务器最后一次知道的任期号（初始化为 0，持续递增）
            std::uint32_t currentTerm_;
            // 当前获取选票的候选人 Id
            boost::optional<std::uint32_t> votedFor_;
            // 日志条目集；每一个条目包含一个用户状态机执行的指令，和收到时的任期号
            std::deque<LogEntry> entries_;

            // 所有服务器上经常变的

            // 最后被应用到状态机的日志条目索引值（初始化为 0，持续递增）
            std::uint64_t lastApplied_;
            // 领导者Id
            boost::optional<std::uint32_t> leaderId_;
            // 选举超时时间
            std::uint32_t electionTimeout_;
            // 投票超时时间
            std::pair<std::uint32_t, std::uint32_t> voteTimeout_;

            // 状态机相关

            // 当前时间戳
            std::int64_t nowTime_;
            // 当前状态
            std::shared_ptr<RaftState> currentState_;
            State currentEnumState_;
            // 下一个状态
            State nextEnumState_;
        };
    }
}

#endif