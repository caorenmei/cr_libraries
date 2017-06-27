#include <cr/raft/replay.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        Replay::Replay(RaftEngine& engine)
            : RaftState(engine)
        {}

        Replay::~Replay()
        {}

        void Replay::onEnter(std::shared_ptr<RaftState> prevState)
        {
            CR_ASSERT(prevState == nullptr);
        }

        void Replay::onLeave()
        {

        }

        std::int64_t Replay::update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            return nowTime;
        }

        std::int64_t Replay::update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages)
        {
            return update(nowTime, outMessages);
        }
    }
}