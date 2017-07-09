#ifndef CR_RAFT_CANDIDATE_H_
#define CR_RAFT_CANDIDATE_H_

#include <set>
#include <map>

#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        class Candidate : public RaftState
        {
        public:

            explicit Candidate(RaftEngine& engine);

            ~Candidate();

            virtual int getState() const override;

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            void updateNextElectionTime(std::uint64_t nowTime);

            bool checkElectionTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            void processVoteReq(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onVoteRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool checkVoteGranted(std::uint64_t nowTime);

            void setNewerTerm(std::uint64_t newerTerm);

            std::uint64_t nextElectionTime_;
            std::set<std::uint64_t> grantNodeIds_;
        };
    }
}

#endif
