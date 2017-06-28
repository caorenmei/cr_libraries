#ifndef CR_RAFT_CANDIDATE_H_
#define CR_RAFT_CANDIDATE_H_

#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        /** 候选者状态 */
        class Candidate : public RaftState
        {
        public:

            explicit Candidate(RaftEngine& engine);

            ~Candidate();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            virtual std::int64_t update(std::int64_t nowTime, RaftMsgPtr inMessage, std::vector<RaftMsgPtr>& outMessages) override;

        private:

        };
    }
}

#endif
