#ifndef CR_RAFT_FOLLOWER_H_
#define CR_RAFT_FOLLOWER_H_

#include <utility>

#include <cr/raft/raft_state.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {

        class Follower : public RaftState
        {
        public:

            explicit Follower(RaftEngine& engine);

            ~Follower();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

        private:

            void updateNextElectionTime(std::uint64_t nowTime);

            bool checkElectionTimeout(std::uint64_t nowTime);

            void processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            void onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr raftMsg, std::vector<RaftMsgPtr>& outMessages);

            bool checkLeaderTerm(std::uint32_t leaderId, const pb::LogAppendReq& request);

            void updateLeaderId(std::uint32_t leaderId, const pb::LogAppendReq& request);

            bool checkPrevLogTerm(std::uint32_t leaderId, const pb::LogAppendReq& request);

            void appendLog(std::uint32_t leaderId, const pb::LogAppendReq& request);

            void updateCommitIndex(std::uint32_t leaderId, const pb::LogAppendReq& request);

            void logAppendResp(std::uint32_t leaderId, bool success, std::vector<RaftMsgPtr>& outMessages);

            void onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr raftMsg, std::vector<RaftMsgPtr>& outMessages);

            void voteResp(std::uint32_t candidateId, const pb::VoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages);

            void setNewerTerm(std::uint32_t newerTerm);

            std::uint64_t nextElectionTime_;
        };
    }
}

#endif
