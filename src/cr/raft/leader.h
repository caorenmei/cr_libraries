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

            struct BuddyNode;

            explicit Leader(RaftEngine& engine);

            ~Leader();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            void updateNextUpdateTime(BuddyNode& BuddyNode, std::uint64_t nowTime);

            std::uint64_t checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onLogAppendRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            bool updateCommitIndex();

            void processLogAppend(std::uint64_t nowTime, bool newerCommitIndex, std::vector<RaftMsgPtr>& outMessages);

            void logAppendReq(BuddyNode& BuddyNode, std::vector<RaftMsgPtr>& outMessages);

            void setNewerTerm(std::uint32_t newerTerm);

            struct BuddyNode
            {
                std::uint32_t nodeId;
                std::uint64_t nextUpdateTime;
                std::uint64_t nextLogIndex;
                std::uint64_t replyLogIndex;
                std::uint64_t matchLogIndex;
            };
            std::map<std::uint32_t, BuddyNode> nodes_;
        };
    }
}

#endif
