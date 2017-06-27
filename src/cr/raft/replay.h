#ifndef CR_RAFT_REPLAY_H_
#define CR_RAFT_REPLAY_H_

#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        /** 日志重放状态 */
        class Replay : public RaftState
        {
        public:

            /**
             * 构造函数
             * @param engine Raft引擎
             */
            explicit Replay(RaftEngine& engine);

            /** 析构函数 */
            ~Replay();

            /**
             * 进入状态调用
             * @param prevState 上一个状态，null为初始状态
             */
            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            /**
             * 离开状态调用
             */
            virtual void onLeave() override;

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            virtual std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            /**
             * 执行状态机逻辑
             * @param nowTime 当前时间戳, ms
             * @param inMessage 输入消息
             * @param outMessages 输出消息
             * @return 下一次update的时间戳, ms
             */
            virtual std::int64_t update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages) override;

        };
    }
}

#endif
