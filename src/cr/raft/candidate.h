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

            explicit Candidate(Raft& raft);

            ~Candidate();

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

            // 检查选举超时,超时返回True
            bool checkElectionTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 发起投票请求
            void processRequestVoteReq(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 处理消息，如果状态转换，则返回True
            bool processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 处理附加日志请求
            bool onAppendEntriesReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            // 处理投票请求
            bool onRequestVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            // 处理请求投票回执
            bool onRequestVoteRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            // 是否选举成功
            bool checkVoteGranted(std::uint64_t nowTime);

            // 选举超时时间
            std::uint64_t nextElectionTime_;
            // 拥有的选票
            std::set<std::uint64_t> grantNodeIds_;
        };
    }
}

#endif
