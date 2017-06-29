#include <cr/raft/follower.h>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {
        Follower::Follower(RaftEngine& engine)
            : RaftState(engine),
            lastHeartbeatTime_(0)
        {}

        Follower::~Follower()
        {}

        void Follower::onEnter(std::shared_ptr<RaftState> prevState)
        {
            lastHeartbeatTime_ = getEngine().getNowTime();
        }

        void Follower::onLeave()
        {

        }

        std::int64_t Follower::update(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            // 选举超时
            if (lastHeartbeatTime_ + getEngine().getElectionTimeout() <= nowTime)
            {
                getEngine().setNextState(RaftEngine::CANDIDATE);
                return nowTime;
            }

            // 处理消息
            dispatchMessage(nowTime, outMessages);

            // 选举超时时间之前需要update
            return std::max<std::int64_t>(lastHeartbeatTime_ + getEngine().getElectionTimeout() / 2, nowTime);
        }

        void Follower::dispatchMessage(std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto message = getEngine().popMessage();
            if (message != nullptr)
            {
                switch (message->msg_type())
                {
                case pb::RaftMsg::LOG_APPEND_REQ:
                {
                    pb::LogAppendReq request;
                    if (message->msg().UnpackTo(&request))
                    {
                        onLogAppendReqHandler(request, nowTime, outMessages);
                    }
                    break;
                }
                case pb::RaftMsg::VOTE_REQ:
                {
                    pb::VoteReq request;
                    if (message->msg().UnpackTo(&request))
                    {
                        onVoteReqHandler(request, nowTime, outMessages);
                    }
                    break;
                }
                default:
                {

                }
                }
            }
        }

        // 追加日志
        void Follower::onLogAppendReqHandler(const pb::LogAppendReq& request, std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            lastHeartbeatTime_ = nowTime;
        }

        // 请求投票
        void Follower::onVoteReqHandler(const pb::VoteReq& request, std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {

        }
    }
}