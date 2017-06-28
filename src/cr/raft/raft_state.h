#ifndef CR_RAFT_RAFT_STATE_H_
#define CR_RAFT_RAFT_STATE_H_

#include <cstdint>
#include <memory>
#include <vector>

namespace cr
{
    namespace raft
    {

        namespace pb
        {
            /** raft 通信协议 */
            class RaftMsg;
        }

        /** Raft引擎 */
        class RaftEngine;

        /** Raft状态机基类 */
        class RaftState
        {
        public:

            /** raft消息包 */
            using RaftMsgPtr = std::shared_ptr<pb::RaftMsg>;

            /**
             * 构造函数
             * @param engine Raft引擎
             */
            explicit RaftState(RaftEngine& engine);

            /** 析构函数 */
            virtual ~RaftState();

            /**
             * 进入状态调用
             * @param prevState 上一个状态，null为初始状态
             */
            virtual void onEnter(std::shared_ptr<RaftState> prevState) = 0;

            /**
             * 离开状态调用
             */
            virtual void onLeave() = 0;

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            virtual std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param inMessage 输入消息
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            virtual std::int64_t update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages) = 0;

            /**
             * 获取算法引擎
             * @return Raft引擎
             */
            RaftEngine& getEngine();

        private:

            // Raft引擎
            RaftEngine& engine_;
        };
    }
}

#endif
