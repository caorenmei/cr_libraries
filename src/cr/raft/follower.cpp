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
            nextElectionTime_(0)
        {}

        Follower::~Follower()
        {}

        int Follower::getState() const
        {
            return RaftEngine::FOLLOWER;
        }

        void Follower::onEnter(std::shared_ptr<RaftState> prevState)
        {
            auto nowTime = engine.getNowTime();
            updateNextElectionTime(nowTime);
        }

        void Follower::onLeave()
        {}

        std::uint64_t Follower::update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto nextUpdateTime = nowTime;
            // 如果没有选举超时
            if (!checkElectionTimeout(nowTime))
            {
                // 则处理消息
                processOneMessage(nowTime, outMessages);
                if (engine.getMessageQueue().empty())
                {
                    nextUpdateTime = nextElectionTime_ ;
                }
            }
            return nextUpdateTime;
        }

        void Follower::updateNextElectionTime(std::uint64_t nowTime)
        {
            nextElectionTime_ = nowTime + engine.randElectionTimeout();
        }

        bool Follower::checkElectionTimeout(std::uint64_t nowTime)
        {
            if (nextElectionTime_ <= nowTime)
            {
                engine.setNextState(RaftEngine::CANDIDATE);
                engine.setLeaderId(boost::none);
                engine.setVotedFor(boost::none);
                return true;
            }
            return false;
        }

        void Follower::processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            if (!engine.getMessageQueue().empty())
            {
                auto message = std::move(engine.getMessageQueue().front());
                engine.getMessageQueue().pop_front();
                CR_ASSERT(message->dest_node_id() == engine.getNodeId())(message->dest_node_id())(engine.getNodeId());
                CR_ASSERT(engine.isBuddyNodeId(message->from_node_id()))(message->from_node_id());
                switch (message->msg_type())
                {
                case pb::RaftMsg::APPEND_ENTRIES_REQ:
                    onAppendEntriesReqHandler(nowTime, std::move(message), outMessages);
                    break;
                case pb::RaftMsg::REQUEST_VOTE_REQ:
                    onRequestVoteReqHandler(nowTime, std::move(message), outMessages);
                    break;
                default:
                    break;
                }
            }
        }

        void Follower::onAppendEntriesReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto leaderId = message->from_node_id();
            CR_ASSERT(message->has_append_entries_req());
            auto& request = message->append_entries_req();
            bool success = false;
            // 如果比当前任期大，则更新任期
            auto currentTerm = engine.getCurrentTerm();
            if (request.leader_term() > currentTerm)
            {
                currentTerm = request.leader_term();
                engine.setCurrentTerm(currentTerm);
                engine.setVotedFor(boost::none);
            }
            // 任期有效，处理附加日志
            if (request.leader_term() != 0 && request.leader_term() == currentTerm)
            {
                // 更新超时时间
                updateNextElectionTime(nowTime);
                // 更新领导者
                auto currentLeaderId = engine.getLeaderId();
                if (!currentLeaderId || leaderId != *currentLeaderId)
                {
                    currentLeaderId = leaderId;
                    engine.setVotedFor(boost::none);
                    engine.setLeaderId(currentLeaderId);
                }
                // 匹配日志
                auto lastLogIndex = engine.getStorage()->getLastIndex();
                if (request.prev_log_index() < lastLogIndex)
                {
                    lastLogIndex = request.prev_log_index();
                    engine.getStorage()->remove(lastLogIndex + 1);
                }
                auto lastLogTerm = engine.getStorage()->getLastTerm();
                if (request.prev_log_index() == lastLogIndex && lastLogIndex != 0 && request.prev_log_term() != lastLogTerm)
                {
                    engine.getStorage()->remove(lastLogIndex);
                    lastLogIndex = engine.getStorage()->getLastIndex();
                    lastLogTerm = engine.getStorage()->getLastTerm();
                }
                // 日志匹配，则应用到状态机
                if (request.prev_log_index() == lastLogIndex && request.prev_log_term() == lastLogTerm)
                {
                    // 追加日志
                    auto logIndex = request.prev_log_index();
                    std::vector<Entry> entries;
                    for (int i = 0; i != request.entries_size(); ++i)
                    {
                        logIndex = logIndex + 1;
                        entries.emplace_back(logIndex, currentTerm, request.entries(i));
                    }
                    engine.getStorage()->append(entries);
                    lastLogIndex = engine.getStorage()->getLastIndex();
                    // 更新已提交日志索引
                    auto commitIndex = engine.getCommitIndex();
                    if (request.leader_commit() > commitIndex && commitIndex < lastLogIndex)
                    {
                        commitIndex = std::min(request.leader_commit(), lastLogIndex);
                        engine.setCommitIndex(commitIndex);
                    }
                    // ok了
                    success = true;
                }
            }
            // 回复消息
            appendEntriesResp(leaderId, success, outMessages);
        }

        void Follower::appendEntriesResp(std::uint64_t leaderId, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto lastLogIndex = engine.getStorage()->getLastIndex();

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(leaderId);
            raftMsg->set_msg_type(pb::RaftMsg::APPEND_ENTRIES_RESP);

            auto& response = *(raftMsg->mutable_append_entries_resp());
            response.set_follower_term(currentTerm);
            response.set_last_log_index(lastLogIndex);
            response.set_success(success);

            outMessages.push_back(std::move(raftMsg));
        }

        void Follower::onRequestVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto candidateId = message->from_node_id();
            auto currentTerm = engine.getCurrentTerm();
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            auto lastLogTerm = engine.getStorage()->getLastTerm();

            CR_ASSERT(message->has_request_vote_req());
            auto& request = message->request_vote_req();

            bool success = false;
            if ((request.candidate_term() >= currentTerm && request.candidate_term() > 0)
                && (std::make_tuple(request.last_log_term(), request.last_log_index()) >= std::make_tuple(lastLogTerm, lastLogIndex)))
            {
                if (currentTerm < request.candidate_term())
                {
                    currentTerm = request.candidate_term();
                    engine.setCurrentTerm(currentTerm);
                    engine.setVotedFor(boost::none);
                }
                auto voteFor = engine.getVotedFor();
                if (!voteFor)
                {
                    voteFor = candidateId;
                    engine.setVotedFor(voteFor);
                }
                success = (*voteFor == candidateId);
            }
            updateNextElectionTime(nowTime);
            requestVoteResp(candidateId, request, success, outMessages);
        }

        void Follower::requestVoteResp(std::uint64_t candidateId, const pb::RequestVoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            RaftMsgPtr raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(candidateId);
            raftMsg->set_msg_type(pb::RaftMsg::REQUEST_VOTE_RESP);

            auto& response = *(raftMsg->mutable_request_vote_resp());
            response.set_success(success);
            response.set_follower_term(engine.getCurrentTerm());

            outMessages.push_back(std::move(raftMsg));
        }
    }
}