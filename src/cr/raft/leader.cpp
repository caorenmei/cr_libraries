#include <cr/raft/leader.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        Leader::Leader(RaftEngine& engine)
            : RaftState(engine)
        {}

        Leader::~Leader()
        {}

        void Leader::onEnter(std::shared_ptr<RaftState> prevState)
        {

        }

        void Leader::onLeave()
        {

        }

        std::int64_t Leader::update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages)
        {
            return nowTime;
        }
    }
}