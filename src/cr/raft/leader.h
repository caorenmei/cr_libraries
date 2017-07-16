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

            explicit Leader(Raft& raft);

            ~Leader();

            // 当前状态
            virtual int getState() const override;

            // 进入状态
            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            // 离开状态
            virtual void onLeave() override;

            // 状态机逻辑
            virtual std::uint64_t update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            // 处理消息, 发生状态转换返回True
            bool processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 处理其它领导者的附加日志请求
            bool onAppendEntriesReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            // 处理附加日志回复
            bool onAppendEntriesRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            // 处理请求投票消息
            bool onRequestVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages);

            // 计算已提交日志索引
            std::uint64_t calNewCommitIndex();

            // 发起附加日志请求
            std::uint64_t processAppendEntriesReq(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 附加日志消息
            void appendEntriesReq(BuddyNode& node, std::vector<RaftMsgPtr>& outMessages);

            // 伙伴节点
            struct BuddyNode
            {
                // 节点Id
                std::uint64_t nodeId;
                // 下次Update时间
                std::uint64_t nextUpdateTime;
                // 下一个日志索引
                std::uint64_t nextLogIndex;
                // 以应答的最后索引
                std::uint64_t replyLogIndex;
                // 匹配上的日志索引
                std::uint64_t matchLogIndex;
            };
            // 节点列表
            std::map<std::uint64_t, BuddyNode> nodes_;
            // 用于计算CommitIndex
            std::vector<std::uint64_t> matchLogIndexs_;
        };
    }
}

#endif
