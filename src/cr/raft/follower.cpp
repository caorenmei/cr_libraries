#include <cr/raft/follower.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        Follower::Follower(RaftEngine& engine)
            : RaftState(engine)
        {}

        Follower::~Follower()
        {}

        void Follower::onEnter(std::shared_ptr<RaftState> prevState)
        {
            
        }

        void Follower::onLeave()
        {

        }

        std::int64_t Follower::update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages)
        {
            return nowTime;
        }
    }
}