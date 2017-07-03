#ifndef CR_RAFT_LEADER_H_
#define CR_RAFT_LEADER_H_

#include <map>

#include <cr/raft/raft_state.h>

namespace cr
{
    namespace raft
    {
        /** 领导者状态 */
        class Leader : public RaftState
        {
        public:

            explicit Leader(RaftEngine& engine);

            ~Leader();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

        private:

            std::uint64_t updateNextHeartbeatTime(std::uint32_t buddyNodeId, std::uint64_t nowTime);

            std::uint64_t checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            void updateCommitIndex();

            void processLogAppend(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            void logAppendReq(std::uint32_t buddyNodeId, std::vector<RaftMsgPtr>& outMessages);

            void setNewerTerm(std::uint32_t newerTerm);

            std::map<std::uint32_t, std::uint64_t> nextHeartbeatTime_;
            std::map<std::uint32_t, std::uint64_t> nextLogIndexs_;
            std::map<std::uint32_t, std::uint64_t> relayLogIndexs_;
            std::map<std::uint32_t, std::uint64_t> matchLogIndexs_;
        };
    }
}

#endif
