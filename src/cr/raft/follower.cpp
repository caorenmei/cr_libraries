#include "follower.h"

#include <tuple>

#include <cr/common/assert.h>

#include "raft.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {
        Follower::Follower(Raft& raft)
            : RaftState(raft),
            nextElectionTime_(0)
        {}

        Follower::~Follower()
        {}

        int Follower::getState() const
        {
            return Raft::FOLLOWER;
        }

        void Follower::onEnter(std::shared_ptr<RaftState> prevState)
        {
            auto nowTime = raft.getNowTime();
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
                if (raft.getMessageQueue().empty())
                {
                    nextUpdateTime = nextElectionTime_ ;
                }
            }
            return nextUpdateTime;
        }

        void Follower::updateNextElectionTime(std::uint64_t nowTime)
        {
            nextElectionTime_ = nowTime + raft.randElectionTimeout();
        }

        bool Follower::checkElectionTimeout(std::uint64_t nowTime)
        {
            if (nextElectionTime_ <= nowTime)
            {
                raft.setNextState(Raft::CANDIDATE);
                raft.setLeaderId(boost::none);
                raft.setVotedFor(boost::none);
                return true;
            }
            return false;
        }

        void Follower::processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            if (!raft.getMessageQueue().empty())
            {
                auto message = std::move(raft.getMessageQueue().front());
                raft.getMessageQueue().pop_front();
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
            auto& request = message->append_entries_req();
            bool success = false;
            // 如果比当前任期大，则更新任期
            auto currentTerm = raft.getCurrentTerm();
            if (request.leader_term() > currentTerm)
            {
                currentTerm = request.leader_term();
                raft.setCurrentTerm(currentTerm);
                raft.setVotedFor(boost::none);
            }
            // 任期有效，处理附加日志
            if (request.leader_term() != 0 && request.leader_term() == currentTerm)
            {
                // 更新超时时间
                updateNextElectionTime(nowTime);
                // 更新领导者
                auto currentLeaderId = raft.getLeaderId();
                if (!currentLeaderId || leaderId != *currentLeaderId)
                {
                    currentLeaderId = leaderId;
                    raft.setVotedFor(boost::none);
                    raft.setLeaderId(currentLeaderId);
                }
                // 匹配日志
                auto lastLogIndex = raft.getStorage()->getLastIndex();
                if (request.prev_log_index() != 0 && request.prev_log_index() <= lastLogIndex)
                {
                    auto prevLogTerm = raft.getStorage()->getTermByIndex(request.prev_log_index());
                    // 起始日志不匹配
                    if (request.prev_log_term() != prevLogTerm)
                    {
                        lastLogIndex = request.prev_log_index() - 1;
                        raft.getStorage()->remove(lastLogIndex + 1);
                    }
                }
                // 匹配附加的日志
                auto leaderLastLogIndex = request.prev_log_index() + request.entries_size();
                auto stopLogIndex = std::min(leaderLastLogIndex, lastLogIndex);
                int leaderEntriesIndex = 0;
                for (auto logIndex = request.prev_log_index() + 1; logIndex <= stopLogIndex; ++logIndex, ++leaderEntriesIndex)
                {
                    auto logTerm = raft.getStorage()->getTermByIndex(logIndex);
                    if (request.entries(leaderEntriesIndex).term() != logTerm)
                    {
                        lastLogIndex = logIndex - 1;
                        raft.getStorage()->remove(lastLogIndex + 1);
                        break;
                    }
                }
                // 日志匹配，则应用到状态机
                if (request.prev_log_index() + leaderEntriesIndex == lastLogIndex && leaderEntriesIndex < request.entries_size())
                {
                    // 追加日志
                    std::vector<pb::Entry> entries(request.entries().begin() + leaderEntriesIndex, request.entries().end());
                    raft.getStorage()->append(lastLogIndex + 1, entries);
                    lastLogIndex = raft.getStorage()->getLastIndex();
                }
                // 日志匹配,更新已提交日志索引
                if (request.prev_log_index() + request.entries_size() <= lastLogIndex)
                {
                    auto commitIndex = raft.getCommitIndex();
                    if (request.leader_commit() > commitIndex && commitIndex < leaderLastLogIndex)
                    {
                        commitIndex = std::min(request.leader_commit(), leaderLastLogIndex);
                        raft.setCommitIndex(commitIndex);
                    }
                    // 日志追加完毕
                    success = true;
                }
            }
            // 回复消息
            appendEntriesResp(leaderId, success, outMessages);
        }

        void Follower::appendEntriesResp(std::uint64_t leaderId, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = raft.getCurrentTerm();
            auto lastLogIndex = raft.getStorage()->getLastIndex();

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(raft.getNodeId());
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
            auto currentTerm = raft.getCurrentTerm();
            auto lastLogIndex = raft.getStorage()->getLastIndex();
            auto lastLogTerm = raft.getStorage()->getLastTerm();

            auto& request = message->request_vote_req();
            bool success = false;
            if ((request.candidate_term() >= currentTerm && request.candidate_term() > 0)
                && (std::make_tuple(request.last_log_term(), request.last_log_index()) >= std::make_tuple(lastLogTerm, lastLogIndex)))
            {
                if (currentTerm < request.candidate_term())
                {
                    currentTerm = request.candidate_term();
                    raft.setCurrentTerm(currentTerm);
                    raft.setVotedFor(boost::none);
                }
                auto voteFor = raft.getVotedFor();
                if (!voteFor)
                {
                    voteFor = candidateId;
                    raft.setVotedFor(voteFor);
                }
                success = (*voteFor == candidateId);
            }
            updateNextElectionTime(nowTime);
            requestVoteResp(candidateId, request, success, outMessages);
        }

        void Follower::requestVoteResp(std::uint64_t candidateId, const pb::RequestVoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            RaftMsgPtr raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(raft.getNodeId());
            raftMsg->set_dest_node_id(candidateId);
            raftMsg->set_msg_type(pb::RaftMsg::REQUEST_VOTE_RESP);

            auto& response = *(raftMsg->mutable_request_vote_resp());
            response.set_success(success);
            response.set_follower_term(raft.getCurrentTerm());

            outMessages.push_back(std::move(raftMsg));
        }
    }
}