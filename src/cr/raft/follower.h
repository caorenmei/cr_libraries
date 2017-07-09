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

            virtual int getState() const override;

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            void updateNextElectionTime(std::uint64_t nowTime);

            bool checkElectionTimeout(std::uint64_t nowTime);

            void processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            void onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr raftMsg, std::vector<RaftMsgPtr>& outMessages);

            bool checkLeaderTerm(std::uint64_t leaderId, const pb::AppendEntriesReq& request);

            void updateLeaderId(std::uint64_t leaderId, const pb::AppendEntriesReq& request);

            bool checkPrevLogTerm(std::uint64_t leaderId, const pb::AppendEntriesReq& request);

            void appendLog(std::uint64_t leaderId, const pb::AppendEntriesReq& request);

            void updateCommitIndex(std::uint64_t leaderId, const pb::AppendEntriesReq& request);

            void logAppendResp(std::uint64_t leaderId, bool success, std::vector<RaftMsgPtr>& outMessages);

            void onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr raftMsg, std::vector<RaftMsgPtr>& outMessages);

            void voteResp(std::uint64_t candidateId, const pb::RequestVoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages);

            void setNewerTerm(std::uint64_t newerTerm);

            std::uint64_t nextElectionTime_;
        };
    }
}

#endif
