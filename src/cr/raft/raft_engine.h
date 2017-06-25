#ifndef CR_RAFT_RAFT_ENGINE_H_
#define CR_RAFT_RAFT_ENGINE_H_

#include <cstdint>
#include <memory>

#include <boost/asio/io_service.hpp>
#include <boost/circular_buffer.hpp>

#include <cr/common/exception.h>
#include <cr/raft/log_storage.h>
#include <cr/raft/state_machine.h>

namespace cr
{
    namespace raft
    {
        /**
         * Raft算法引擎
         * 一个Raft算法引擎对应一个raft实例
         */
        class RaftEngine
        {
        public:

            /** 构造异常 */
            class BuildException : public cr::Exception
            {
            public:
                using cr::Exception::Exception;
            };
           
            /** Raft引擎构造器 */
            class Builder
            {
            public:

                /** 构造函数 */
                Builder();

                /** 析构函数 */
                ~Builder();

                /**
                 * 设置节点Id
                 * @param nodeId 节点Id
                 * @reutrn this
                 */
                Builder& setNodeId(std::uint32_t nodeId);

                /**
                 * 设置实例Id
                 * @param instanceId 实例Id
                 * @reutrn this
                 */
                Builder& setInstanceId(std::uint32_t instanceId);

                /**
                 * 设置其他节点Id
                 * @param otherNodeIds 其它节点Id
                 * @reutrn this
                 */
                Builder& setOtherNodeIds(std::vector<std::uint32_t> otherNodeIds);

                /**
                 * 设置日志存储接口
                 * @param storage 日志存储接口
                 * @reutrn this
                 */
                Builder& setLogStorage(std::shared_ptr<LogStorage> storage);

                /**
                 * 设置状态机
                 * @param stateMachine 状态机
                 */
                Builder& setStateMachine(std::shared_ptr<StateMachine> stateMachine);

                /**
                 * 构造Raft引擎
                 * @return Raft引擎, 非空成功
                 */
                std::shared_ptr<RaftEngine> build();

            private:

                // 本节点ID
                std::uint32_t nodeId_;
                // 实例Id
                std::uint32_t instanceId_;
                // 其它节点ID
                std::vector<std::uint32_t> otherNodeIds_;
                // 日志存储接口
                std::shared_ptr<LogStorage> storage_;
                // 状态机
                std::shared_ptr<StateMachine> stateMachine_;
            };

            /**
             * 构造函数
             * @param nodeId 节点ID
             * @param instanceId 实例ID
             * @param otherNodeIds 其它节点ID
             * @param storage 存储接口
             * @param stateMachine 状态机
             */
            RaftEngine(std::uint32_t nodeId, std::uint32_t instanceId, std::vector<std::uint32_t> otherNodeIds, 
                std::shared_ptr<LogStorage> storage, std::shared_ptr<StateMachine> stateMachine);

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
             * 获取实例Id
             * @return 实例Id
             */
            std::uint32_t getInstanceId() const;

            /**
             * 获取其它节点Id
             * @return 其它节点Id
             */
            const std::vector<std::uint32_t>& getOtherNodeIds() const;

        private:

            // 本节点ID
            std::uint32_t nodeId_;
            // 实例Id
            std::uint32_t instanceId_;
            // 其它节点ID
            std::vector<std::uint32_t> otherNodeIds_;
            // 存储接口
            std::shared_ptr<LogStorage> storage_;
            // 状态机
            std::shared_ptr<StateMachine> stateMachine_;
        };
    }
}

#endif