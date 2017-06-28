#include <cr/raft/candidate.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>

namespace cr
{
    namespace raft
    {
        Candidate::Candidate(RaftEngine& engine)
            : RaftState(engine)
        {}

        Candidate::~Candidate()
        {}

        void Candidate::onEnter(std::shared_ptr<RaftState> prevState)
        {

        }

        void Candidate::onLeave()
        {

        }

        std::int64_t Candidate::update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages)
        {
            return nowTime;
        }
    }
}