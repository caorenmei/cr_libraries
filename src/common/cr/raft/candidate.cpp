#include "candidate.h"

namespace cr
{
    namespace raft
    {

        CandidateState::CandidateState()
            : state_(nullptr),
            electionTime_(0)
        {}

        CandidateState::~CandidateState()
        {}

        void CandidateState::on_entry(const ElectionTimeoutEvent&, RaftState&)
        {

        }

        void CandidateState::on_exit(const ElectionTimeoutEvent&, RaftState&)
        {

        }

        void CandidateState::on_exit(const MajorityVotesEvent&, RaftState&)
        {

        }

        void CandidateState::on_exit(const FinalEvent&, RaftState&)
        {

        }

        void CandidateState::set_sm_ptr(RaftState* sm)
        {
            state_ = sm;
        }

        std::uint64_t CandidateState::update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages) 
        {
            return nowTime;
        }
    }
}