﻿#ifndef CR_RAFT_LEADER_H_
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

            struct node;

            explicit Leader(RaftEngine& engine);

            ~Leader();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            void updateNextHeartbeatTime(node& node, std::uint64_t nowTime);

            std::uint64_t checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            void updateCommitIndex();

            void processLogAppend(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            void logAppendReq(node& node, std::vector<RaftMsgPtr>& outMessages);

            void setNewerTerm(std::uint32_t newerTerm);

            struct node
            {
                std::uint32_t nodeId;
                std::uint64_t nextTickTime;
                std::uint64_t nextLogIndex;
                std::uint64_t replyLogindex;
                std::uint64_t matchLogIndex;
            };
            std::map<std::uint32_t, node> nodes_;
        };
    }
}

#endif
