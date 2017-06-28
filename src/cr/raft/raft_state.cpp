#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        RaftState::RaftState(RaftEngine& engine)
            : engine_(engine)
        {}

        RaftState::~RaftState()
        {}

        RaftEngine& RaftState::getEngine()
        {
            return engine_;
        }

        std::int64_t RaftState::update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            return update(nowTime, nullptr, outMessages);
        }
    }
}