#include "raft_state.h"

namespace cr
{
    namespace raft
    {
        RaftState::RaftState(Raft& raft)
            : raft(raft)
        {}

        RaftState::~RaftState()
        {}
    }
}