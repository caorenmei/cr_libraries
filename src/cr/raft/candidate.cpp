#include "candidate.h"

#include <limits>
#include <random>
#include <tuple>

#include <cr/common/assert.h>

#include "raft.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {

        Candidate::Candidate(Raft& raft)
            : RaftState(raft),
            nextElectionTime_(0)
        {}

        Candidate::~Candidate()
        {}

        int Candidate::getState() const
        {
            return Raft::CANDIDATE;
        }

        void Candidate::onEnter(std::shared_ptr<RaftState> prevState)
        {
            nextElectionTime_ = raft.getNowTime();
        }

        void Candidate::onLeave()
        {}

        std::uint64_t Candidate::update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto nextUpdateTime = nowTime;
            // 如果没有选举自己， 则处理消息
            if (!checkElectionTimeout(nowTime, outMessages))
            {
                if (!processOneMessage(nowTime, outMessages))
                {
                    // 如果没有转换状态，则等待超时
                    if (raft.getMessageQueue().empty())
                    {
                        nextUpdateTime = nextElectionTime_;
                    }
                }
            }
            return nextUpdateTime;
        }

        void Candidate::updateNextElectionTime(std::uint64_t nowTime)
        {
            nextElectionTime_ = nowTime + raft.randElectionTimeout();
        }

        bool Candidate::checkElectionTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            // 如果选举超时
            if (nextElectionTime_ <= nowTime)
            {
                // 重设超时时间
                updateNextElectionTime(nowTime);
                // 增加任期
                raft.setCurrentTerm(raft.getCurrentTerm() + 1);
                // 给自己一票
                grantNodeIds_.clear();
                raft.setVotedFor(raft.getNodeId());
                grantNodeIds_.insert(raft.getNodeId());
                // 发送请求投票消息
                processRequestVoteReq(nowTime, outMessages);
                // 如果只有一个节点，则选举自己
                return checkVoteGranted(nowTime);
            }
            return false;
        }

        void Candidate::processRequestVoteReq(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = raft.getCurrentTerm();
            auto lastLogIndex = raft.getStorage()->getLastIndex();
            auto lastLogTerm = raft.getStorage()->getLastTerm();
            for (auto buddyNodeId : raft.getBuddyNodeIds())
            {
                auto raftMsg = std::make_shared<pb::RaftMsg>();
                raftMsg->set_from_node_id(raft.getNodeId());
                raftMsg->set_dest_node_id(buddyNodeId);

                auto& request = *(raftMsg->mutable_request_vote_req());
                request.set_candidate_term(currentTerm);
                request.set_last_log_index(lastLogIndex);
                request.set_last_log_term(lastLogTerm);

                outMessages.push_back(std::move(raftMsg));
            }
        }

        bool Candidate::processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            if (!raft.getMessageQueue().empty())
            {
                auto message = std::move(raft.getMessageQueue().front());
                raft.getMessageQueue().pop_front();
                if (message->has_append_entries_req())
                {
                    return onAppendEntriesReqHandler(nowTime, std::move(message), outMessages);
                }
                else if (message->has_request_vote_req())
                {
                    return onRequestVoteReqHandler(nowTime, std::move(message), outMessages);
                }
                else if (message->has_request_vote_resp())
                {
                    return onRequestVoteRespHandler(nowTime, std::move(message), outMessages);
                }
            }
            return false;
        }

        bool Candidate::onAppendEntriesReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto& request = message->append_entries_req();
            // 如果领导者任期符合,则转换到跟随者
            if (raft.getCurrentTerm() <= request.leader_term())
            {
                raft.getMessageQueue().push_front(std::move(message));
                raft.setNextState(Raft::FOLLOWER);
                raft.setVotedFor(boost::none);
                return true;
            }
            return false;
        }

        bool Candidate::onRequestVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto& request = message->request_vote_req();
            // 如果对方任期比自己大，则转换到跟随者
            if (raft.getCurrentTerm() < request.candidate_term())
            {
                raft.getMessageQueue().push_front(std::move(message));
                raft.setNextState(Raft::FOLLOWER);
                raft.setVotedFor(boost::none);
                return true;
            }
            return false;
        }

        bool Candidate::onRequestVoteRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto& response = message->request_vote_resp();
            auto followerId = message->from_node_id();
            auto currentTerm = raft.getCurrentTerm();
            // 是本次投票结果,判断是否选举成功，成功转换到领导者
            if (currentTerm == response.follower_term() && response.success())
            {
                grantNodeIds_.insert(followerId);
                return checkVoteGranted(nowTime);
            }
            // 比跟随者任期小，更新自己的任期，转化到跟随者
            else if (currentTerm < response.follower_term())
            {
                currentTerm = response.follower_term();
                raft.setCurrentTerm(currentTerm);
                raft.setNextState(Raft::FOLLOWER);
                raft.setVotedFor(boost::none);
                return true;
            }
            return false;
        }

        bool Candidate::checkVoteGranted(std::uint64_t nowTime)
        {
            // 超过半数选票，选举成功,转换到领导者
            if (grantNodeIds_.size() > (1 + raft.getBuddyNodeIds().size()) / 2)
            {
                raft.setNextState(Raft::LEADER);
                raft.setLeaderId(raft.getNodeId());
                raft.setVotedFor(boost::none);
                return true;
            }
            return false;
        }
    }
}