#ifndef CR_COMMON_RAFT_FOLLOWER_STATE_H_
#define CR_COMMON_RAFT_FOLLOWER_STATE_H_

#include <utility>
#include "raft_state.h"

namespace cr
{
    namespace raft
    {
        namespace pb
        {
            /** 追加日志消息 */
            class RequestVoteReq;
        }

        /** 跟随状态 */
        class FollowerState : public boost::msm::front::state<BaseState, boost::msm::front::sm_ptr>
        {
        public:

            /** 构造函数 */
            FollowerState();

            /** 析构函数 */
            ~FollowerState();

            /* 初始化 */
            void on_entry(const StartUpEvent&, RaftState&);

            /* 成为跟随者 */
            void on_entry(const DiscoversEvent&, RaftState&);

            /* 离开状态 */
            void on_exit(const ElectionTimeoutEvent&, RaftState&);

            /* 离开状态 */
            void on_exit(const FinalEvent&, RaftState&);

            /* 设置状态机 */
            void set_sm_ptr(RaftState* sm);

            /**
             * 状态逻辑处理
             * @param nowTime 当前时间
             * @param messages 输出消息
             * @return 下一次需要update的时间
             */
            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages) override;

        private:

            // 状态机
            RaftState* state_;
            // 下一个超时时间
            std::uint64_t electionTime_;
        };
    }
}

#endif
