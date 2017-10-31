#include "leader.h"

namespace cr
{
    namespace raft
    {
        LeaderState::LeaderState()
            : state_(nullptr)
        {}

        LeaderState::~LeaderState()
        {}

        void LeaderState::on_entry(const MajorityVotesEvent&, RaftState&)
        {

        }

        void LeaderState::on_exit(const DiscoversEvent&, RaftState&)
        {

        }

        void LeaderState::on_exit(const FinalEvent&, RaftState&)
        {

        }

        void LeaderState::set_sm_ptr(RaftState* sm)
        {
            state_ = sm;
        }

        std::uint64_t LeaderState::update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return nowTime;
        }
    }
}