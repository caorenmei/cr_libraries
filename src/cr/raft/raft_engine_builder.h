#ifndef CR_RAFT_RAFT_ENGINE_BUILDER_H_
#define CR_RAFT_RAFT_ENGINE_BUILDER_H_

#include <cstdint>
#include <memory>
#include <utility>

#include <boost/optional.hpp>

#include <cr/raft/log_storage.h>
#include <cr/raft/state_machine.h>

namespace cr
{
    namespace raft
    {
        /** Raft算法引擎 */
        class RaftEngine;

        /** Raft引擎构造器 */
        class RaftEngineBuilder
        {
        public:

            /** 构造函数 */
            RaftEngineBuilder();

            /** 析构函数 */
            ~RaftEngineBuilder();

            /**
             * 设置节点Id
             * @param nodeId 节点Id
             * @reutrn this
             */
            RaftEngineBuilder& setNodeId(std::uint32_t nodeId);

            /**
             * 获取节点Id
             * @param 节点Id
             */
            std::uint32_t getNodeId() const;

            /**
             * 设置其他节点Id
             * @param otherNodeIds 其它节点Id
             * @reutrn this
             */
            RaftEngineBuilder& setOtherNodeIds(std::vector<std::uint32_t> otherNodeIds);

            /**
             * 获取其它节点Id
             * @return 其它节点Id
             */
            const std::vector<std::uint32_t>& getOtherNodeIds() const;

            /**
             * 设置日志存储接口
             * @param storage 日志存储接口
             * @reutrn this
             */
            RaftEngineBuilder& setLogStorage(std::shared_ptr<LogStorage> storage);

            /**
             * 获取日志存储接口
             * @return 日志存储接口
             */
            const std::shared_ptr<LogStorage>& getLogStorage() const;

            /**
             * 设置状态机
             * @param stateMachine 状态机
             */
            RaftEngineBuilder& setStateMachine(std::shared_ptr<StateMachine> stateMachine);

            /**
             * 获取状态机接口
             * @return 状态机接口
             */
            const std::shared_ptr<StateMachine>& getStateMachine() const;

            /**
             * 设置选举超时时间
             * @param electionTimeout 选举超时时间，毫秒
             */
            RaftEngineBuilder& setElectionTimeout(std::uint32_t electionTimeout);

            /**
             * 获取选举超时时间
             * @return 选举超时时间，毫秒
             */
            std::uint32_t getElectionTimeout() const;

            /**
             * 设置投票超时时间
             * @param electionTimeout 投票超时时间，毫秒
             */
            RaftEngineBuilder& setVoteTimeout(const std::pair<std::uint32_t, std::uint32_t>& voteTimeout);

            /**
             * 获取投票超时时间
             * @return 投票超时时间，毫秒
             */
            const std::pair<std::uint32_t, std::uint32_t>& getVoteTimeout() const;

            /**
             * 构造Raft引擎
             * @return Raft引擎, 非空成功
             */
            std::shared_ptr<RaftEngine> build();

        private:

            // 本节点ID
            std::uint32_t nodeId_;
            // 其它节点ID
            std::vector<std::uint32_t> otherNodeIds_;
            // 日志存储接口
            std::shared_ptr<LogStorage> storage_;
            // 状态机
            std::shared_ptr<StateMachine> stateMachine_;
            // 超时时间
            std::uint32_t electionTimeout_;
            // 投票超时时间
            std::pair<std::uint32_t, std::uint32_t> voteTimeout_;
        };
    }
}

#endif


