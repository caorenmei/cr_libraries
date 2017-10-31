#ifndef CR_COMMON_RAFT_LEADER_H_
#define CR_COMMON_RAFT_LEADER_H_

#include "raft_state.h"

namespace cr
{
    namespace raft
    {
        /** 领导则状态 */
        class LeaderState : public boost::msm::front::state<BaseState, boost::msm::front::sm_ptr>
        {
        public:

            /** 构造函数 */
            LeaderState();

            /** 析构函数 */
            ~LeaderState();

            /* 得到多数选票 */
            void on_entry(const MajorityVotesEvent&, RaftState&);

            /* 其它节点成为领导者 */
            void on_exit(const DiscoversEvent&, RaftState&);

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
        };
    }
}

#endif
