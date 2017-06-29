#ifndef CR_RAFT_FOLLOWER_H_
#define CR_RAFT_FOLLOWER_H_

#include <cr/raft/raft_state.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {
        /** 跟随者状态 */
        class Follower : public RaftState
        {
        public:

            explicit Follower(RaftEngine& engine);

            ~Follower();

            virtual void onEnter(std::shared_ptr<RaftState> prevState) override;

            virtual void onLeave() override;

            virtual std::int64_t update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages) override;

            // 最后的心跳时间
            std::int64_t getLastHeartbeatTime() const;

        private:

            // 消息处理
            void dispatchMessage(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 追加日志
            void onLogAppendReqHandler(const pb::LogAppendReq& request, std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 请求投票
            void onVoteReqHandler(const pb::VoteReq& request, std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages);

            // 最后心跳时间
            std::int64_t lastHeartbeatTime_;

        };
    }
}

#endif
