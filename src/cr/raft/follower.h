#ifndef CR_RAFT_FOLLOWER_H_
#define CR_RAFT_FOLLOWER_H_

#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        /** 跟随者状态 */
        class Follower : public RaftState
        {
        public:

            explicit Follower(RaftEngine& engine);

            ~Follower();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            virtual std::int64_t update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages) override;

        private:

        };
    }
}

#endif
