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
            : raft_(raft),
            nowTime_(0)
        {}

        RaftState_::~RaftState_()
        {}

        void RaftState_::on_entry(const StartUpEvent&, RaftState&)
        {

        }

        void RaftState_::on_exit(const StartUpEvent&, RaftState&)
        {

        }

        std::uint64_t RaftState_::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return nowTime_;
        }

        Raft& RaftState_::getRaft()
        {
            return raft_;
        }

        void RaftState_::setNowTime(std::uint64_t nowTime)
        {
            nowTime_ = nowTime;
        }

        std::uint64_t RaftState_::getNowTime() const
        {
            return nowTime_;
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

        std::uint64_t FollowerState::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return state_->getNowTime();
        }

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

        std::uint64_t CandidateState::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return state_->getNowTime();
        }

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

        std::uint64_t LeaderState::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            return state_->getNowTime();
        }

    }
}