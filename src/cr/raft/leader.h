#ifndef CR_RAFT_LEADER_H_
#define CR_RAFT_LEADER_H_

#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        /** 领导者状态 */
        class Leader : public RaftState
        {
        public:

            explicit Leader(RaftEngine& engine);

            ~Leader();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

        private:

        };
    }
}

#endif
