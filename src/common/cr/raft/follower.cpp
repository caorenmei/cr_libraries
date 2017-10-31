#include "follower.h"


namespace cr
{
    namespace raft
    {

        FollowerState::FollowerState()
            : state_(nullptr),
            electionTime_(0)
        {}

        FollowerState::~FollowerState()
        {}

        void FollowerState::on_entry(const StartUpEvent&, RaftState&)
        {

        }

        void FollowerState::on_entry(const DiscoversEvent&, RaftState&)
        {

        }

        void FollowerState::on_exit(const ElectionTimeoutEvent&, RaftState&)
        {

        }

        void FollowerState::on_exit(const FinalEvent&, RaftState&)
        {

        }

        void FollowerState::set_sm_ptr(RaftState* sm)
        {
            state_ = sm;
        }

        std::uint64_t FollowerState::update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return nowTime;
        }
    }
}