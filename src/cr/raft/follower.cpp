#include <cr/raft/follower.h>

#include <tuple>

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
            auto electionTimeout = getEngine().getElectionTimeout();
            // 选举超时
            if (lastHeartbeatTime_ + electionTimeout <= nowTime)
            {
                getEngine().setNextState(RaftEngine::CANDIDATE);
                return nowTime;
            }
            // 处理消息
            dispatchMessage(nowTime, outMessages);
            // 选举超时时间之前需要update
            return std::max<std::int64_t>(lastHeartbeatTime_ + electionTimeout / 2, nowTime);
        }

        std::int64_t Follower::getLastHeartbeatTime() const
        {
            return lastHeartbeatTime_;
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
            // 不是伙伴Id
            if (!getEngine().isBuddyNodeId(request.candidate_id()))
            {
                return;
            }
            // 本节点状态
            std::uint32_t currentTerm = getEngine().getCurrentTerm();
            std::uint64_t lastLogIndex = getEngine().getLogStorage()->getLastIndex();
            std::uint32_t lastLogTerm = 0;
            if (lastLogIndex != 0)
            {
                lastLogTerm = getEngine().getLogStorage()->getTermByIndex(lastLogIndex);
            }
            pb::VoteResp voteResp;
            voteResp.set_success(false);
            // 投票
            if (std::make_tuple(request.last_log_term(), request.last_log_index()) >= std::make_tuple(lastLogTerm, lastLogIndex)
                && request.candidate_term() >= currentTerm)
            {
                auto voteFor = getEngine().getVotedFor();
                if (!voteFor || request.candidate_term() > currentTerm)
                {
                    voteFor = request.candidate_id();
                    getEngine().setVotedFor(voteFor);
                }
                if (*voteFor == request.candidate_id())
                {
                    voteResp.set_success(true);
                }
                // 更新任期
                if (currentTerm < request.candidate_term())
                {
                    currentTerm = request.candidate_term();
                    getEngine().setCurrentTerm(currentTerm);
                }
            }
            voteResp.set_follower_id(getEngine().getNodeId());
            voteResp.set_candidate_term(request.candidate_term());
            voteResp.set_follower_term(currentTerm);
            // 应答
            RaftMsgPtr response = std::make_shared<pb::RaftMsg>();
            response->set_node_id(request.candidate_id());
            response->set_msg_type(pb::RaftMsg::VOTE_RESP);
            response->mutable_msg()->PackFrom(voteResp);
            outMessages.push_back(std::move(response));
            // 心跳时间
            lastHeartbeatTime_ = nowTime;
        }
    }
}