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
            // 不是伙伴Id
            if (!getEngine().isBuddyNodeId(request.leader_id()))
            {
                return;
            }
            bool success = false;
            do
            {
                // 任期状态
                if (!checkLeaderTerm(request))
                {
                    break;
                }
                // 心跳
                lastHeartbeatTime_ = nowTime;
                // 领导者Id
                if (!checkLeaderId(request))
                {
                    break;
                }
                // 检查日志一致性
                if (!checkPrevLogTerm(request))
                {
                    break;
                }
                // 应用日志
                if (!appendLog(request))
                {
                    break;
                }
                // success
                success = true;
            } while (0);
            // 回执
            logAppendResp(success, outMessages);
        }

        bool Follower::checkLeaderTerm(const pb::LogAppendReq& request)
        {
            std::uint32_t currentTerm = getEngine().getCurrentTerm();
            if (request.leader_term() < currentTerm)
            {
                return false;
            }
            if (request.leader_term() > currentTerm)
            {
                currentTerm = request.leader_term();
                getEngine().setCurrentTerm(currentTerm);
            }
            return true;
        }

        bool Follower::checkLeaderId(const pb::LogAppendReq& request)
        {
            auto leaderId = getEngine().getLeaderId();
            if (!leaderId || request.leader_id() != *leaderId)
            {
                getEngine().setLeaderId(request.leader_id());
            }
            return true;
        }

        bool Follower::checkPrevLogTerm(const pb::LogAppendReq& request)
        {
            std::uint64_t lastLogIndex = getEngine().getCommitLogIndex();
            if (lastLogIndex >= request.prev_log_index())
            {
                std::uint32_t prevLogTerm = 0;
                if (lastLogIndex > 0)
                {
                    prevLogTerm = getEngine().getLogStorage()->getTermByIndex(request.prev_log_index());
                }
                return prevLogTerm == request.prev_log_term();
            }
            return false;
        }

        bool Follower::appendLog(const pb::LogAppendReq& request)
        {
            std::uint32_t currentTerm = getEngine().getCurrentTerm();
            std::uint64_t logIndex = request.prev_log_index();
            for (int i = 0; i < request.entries_size(); ++i)
            {
                logIndex = logIndex + 1;
                checkAppendLogTerm(logIndex);
                if (logIndex > getEngine().getCommitLogIndex())
                {
                    getEngine().getLogStorage()->append({ logIndex , currentTerm, request.entries(i) });
                }
            }
            return true;
        }

        bool Follower::checkAppendLogTerm(std::uint64_t logIndex)
        {
            if (logIndex <= getEngine().getCommitLogIndex())
            {
                std::uint32_t logTerm = getEngine().getLogStorage()->getTermByIndex(logIndex);
                if (logTerm != getEngine().getCurrentTerm())
                {
                    getEngine().getLogStorage()->del(logIndex);
                }
            }
            return true;
        }

        void Follower::logAppendResp(bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::LogAppendResp response;
            response.set_follower_id(getEngine().getNodeId());
            response.set_follower_term(getEngine().getCurrentTerm());
            response.set_last_log_index(getEngine().getCommitLogIndex());
            response.set_last_log_term(getEngine().getCommitLogTerm());
            response.set_success(success);

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_node_id(*getEngine().getLeaderId());
            raftMsg->set_msg_type(pb::RaftMsg::LOG_APPEND_RESP);
            raftMsg->mutable_msg()->PackFrom(response);

            outMessages.push_back(std::move(raftMsg));
        }

        void Follower::onVoteReqHandler(const pb::VoteReq& request, std::int64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            // 不是伙伴Id
            if (!getEngine().isBuddyNodeId(request.candidate_id()))
            {
                return;
            }
            // 本节点状态
            std::uint32_t currentTerm = getEngine().getCurrentTerm();
            std::uint64_t lastLogIndex = getEngine().getCommitLogIndex();
            std::uint32_t lastLogTerm = getEngine().getCommitLogTerm();
            // 投票
            bool success = false;
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
                    success = true;
                }
                // 更新任期
                if (currentTerm < request.candidate_term())
                {
                    currentTerm = request.candidate_term();
                    getEngine().setCurrentTerm(currentTerm);
                }
            }
            // 心跳时间
            lastHeartbeatTime_ = nowTime;
            // 应答
            voteResp(request, success, outMessages);
        }

        void Follower::voteResp(const pb::VoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::VoteResp response;
            response.set_success(success);
            response.set_follower_id(getEngine().getNodeId());
            response.set_candidate_term(request.candidate_term());
            response.set_follower_term(getEngine().getCurrentTerm());
            // 应答
            RaftMsgPtr raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_node_id(request.candidate_id());
            raftMsg->set_msg_type(pb::RaftMsg::VOTE_RESP);
            raftMsg->mutable_msg()->PackFrom(response);
            outMessages.push_back(std::move(raftMsg));
        }
    }
}