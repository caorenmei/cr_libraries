#ifndef CR_RAFT_FOLLOWER_H_
#define CR_RAFT_FOLLOWER_H_

#include <utility>

#include "raft_state.h"

namespace cr
{
    namespace raft
    {
        namespace pb
        {
            class RequestVoteReq;
        }

        class Follower : public RaftState
        {
        public:

            explicit Follower(Raft& raft);

            ~Follower();

            // 当前状态
            virtual int getState() const override;

            // 进入状态
            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            // 离开状态
            virtual void onLeave() override;

            // 状态机逻辑
            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            // 更新选举超时时间
            void updateNextElectionTime(std::uint64_t nowTime);

            // 是否选举超时
            bool checkElectionTimeout(std::uint64_t nowTime);

            // 处理消息
            void processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 处理领导者的附加日志消息
            void onAppendEntriesReqHandler(std::uint64_t nowTime, RaftMsgPtr raftMsg, std::vector<RaftMsgPtr>& outMessages);

            // 回复附加日志消息
            void appendEntriesResp(std::uint64_t leaderId, bool success, std::vector<RaftMsgPtr>& outMessages);

            // 处理请求投票日志消息
            void onRequestVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr raftMsg, std::vector<RaftMsgPtr>& outMessages);

            // 回复请求投票日志消息
            void requestVoteResp(std::uint64_t candidateId, const pb::RequestVoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages);

            // 下一个超时时间
            std::uint64_t nextElectionTime_;
        };
    }
}

#endif
