#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        RaftState::RaftState(RaftEngine& engine)
            : engine(engine)
        {}

        RaftState::~RaftState()
        {}
    }
}