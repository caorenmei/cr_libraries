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

            explicit Replay(RaftEngine& engine);

            ~Replay();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::int64_t update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages) override;

        };
    }
}

#endif
