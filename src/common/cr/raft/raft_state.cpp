#include "raft_state.h"

namespace cr
{
    namespace raft
    {
        // 跟随者状态
        constexpr std::size_t FOLLOWER_STATE = 0;
        // 候选者状态
        constexpr std::size_t CANDIDATE_STATE = 0;
        // 领导者状态
        constexpr std::size_t LEADER_STATE = 0;
        

        RaftState_::RaftState_(Raft& raft)
            : raft_(raft)
        {}

        RaftState_::~RaftState_()
        {}

        void RaftState_::on_entry(const StartUpEvent&, RaftState&)
        {

        }

        void RaftState_::on_exit(const StartUpEvent&, RaftState&)
        {

        }

        std::uint64_t RaftState_::update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return nowTime;
        }

        Raft& RaftState_::getRaft()
        {
            return raft_;
        }

        bool RaftState_::isFollower() const
        {
            return static_cast<const RaftState*>(this)->current_state()[0] == FOLLOWER_STATE;
        }

        bool RaftState_::isCandidate() const
        {
            return static_cast<const RaftState*>(this)->current_state()[0] == CANDIDATE_STATE;
        }

        bool RaftState_::isLeader() const
        {
            return static_cast<const RaftState*>(this)->current_state()[0] == LEADER_STATE;
        }

    }
}