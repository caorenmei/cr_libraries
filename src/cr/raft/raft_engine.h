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
            const std::vector<std::uint32_t>& getOtherNodeIds() const;

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
             * 获取当前时间戳
             * @param 当前时间戳， 以毫秒计算
             */
            std::int64_t getNowTime() const;

            /**
             * 获取当前状态
             * @param 当前状态
             */
            const std::shared_ptr<RaftState>& getCurrentState() const;

            /**
             * 设置下一个状态
             * @param nextState 下一个状态
             */
            void setNextState(std::shared_ptr<RaftState> nextState);

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param inMessage 输入消息
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            std::int64_t update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages);

        private:
        
            // 状态机为友元类
            friend class Replay;

            // 状态切换
            void onTransitionState();

            // 节点相关数据

            // 本节点ID
            std::uint32_t nodeId_;
            // 其它节点ID
            std::vector<std::uint32_t> otherNodeIds_;
            // 存储接口
            std::shared_ptr<LogStorage> storage_;
            // 状态机
            std::shared_ptr<StateMachine> stateMachine_;

            // 所有服务器上持久存在的

            // 服务器最后一次知道的任期号（初始化为 0，持续递增）
            std::uint32_t currentTerm_;
            // 当前获取选票的候选人 Id
            boost::optional<std::uint32_t> votedFor_;
            // 日志条目集；每一个条目包含一个用户状态机执行的指令，和收到时的任期号
            std::deque<LogEntry> entries_;

            // 所有服务器上经常变的

            // 已知的最大的已经被提交的日志条目的索引值
            std::uint64_t commitLogIndex_;
            // 最后被应用到状态机的日志条目索引值（初始化为 0，持续递增）
            std::uint64_t lastApplied_;

            // 状态机相关

            // 当前时间戳
            std::int64_t nowTime_;
            // 当前状态
            std::shared_ptr<RaftState> currentState_;
            // 下一个状态
            std::shared_ptr<RaftState> nextState_;
        };
    }
}

#endif