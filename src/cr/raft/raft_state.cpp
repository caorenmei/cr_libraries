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
    }
}