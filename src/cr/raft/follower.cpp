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
            if (!checkElectionTimeout(nowTime))
            {
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
            nextElectionTime_ = nowTime + engine.randomElectionTimeout();
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
                case pb::RaftMsg::LOG_APPEND_REQ:
                    onLogAppendReqHandler(nowTime, std::move(message), outMessages);
                    break;
                case pb::RaftMsg::VOTE_REQ:
                    onVoteReqHandler(nowTime, std::move(message), outMessages);
                    break;
                default:
                    break;
                }
            }
        }

        void Follower::onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto leaderId = message->from_node_id();
            CR_ASSERT(message->has_log_append_req());
            auto& request = message->log_append_req();
            bool success = false;
            if (checkLeaderTerm(leaderId, request))
            {
                updateNextElectionTime(nowTime);
                updateLeaderId(leaderId, request);
                if (checkPrevLogTerm(leaderId, request))
                {
                    appendLog(leaderId, request);
                    updateCommitIndex(leaderId, request);
                    success = true;
                }
            }
            logAppendResp(leaderId, success, outMessages);
        }

        bool Follower::checkLeaderTerm(std::uint64_t leaderId, const pb::LogAppendReq& request)
        {
            auto currentTerm = engine.getCurrentTerm();
            if (request.leader_term() > currentTerm)
            {
                currentTerm = request.leader_term();
                setNewerTerm(currentTerm);
            }
            return request.leader_term() != 0 && request.leader_term() == currentTerm;
        }

        void Follower::updateLeaderId(std::uint64_t leaderId, const pb::LogAppendReq& request)
        {
            auto currentLeaderId = engine.getLeaderId();
            if (!currentLeaderId || leaderId != *currentLeaderId)
            {
                engine.setLeaderId(leaderId);
            }
        }

        bool Follower::checkPrevLogTerm(std::uint64_t leaderId, const pb::LogAppendReq& request)
        {
            auto lastLogIndex = engine.getStorage()->lastIndex();
            if (request.prev_log_index() < lastLogIndex)
            {
                lastLogIndex = request.prev_log_index();
                engine.getStorage()->remove(lastLogIndex + 1);
            }
            if (request.prev_log_index() == lastLogIndex && engine.getStorage()->lastTerm() != request.prev_log_term())
            {
                engine.getStorage()->remove(lastLogIndex);
            }
            return request.prev_log_index() == lastLogIndex && request.prev_log_term() == engine.getStorage()->lastTerm();;
        }

        void Follower::appendLog(std::uint64_t leaderId, const pb::LogAppendReq& request)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto logIndex = request.prev_log_index();
            for (int i = 0; i < request.entries_size(); ++i)
            {
                logIndex = logIndex + 1;
                CR_ASSERT(logIndex == engine.getStorage()->lastIndex() + 1);
                engine.getStorage()->append({ logIndex, currentTerm, request.entries(i) });
            }
        }

        void Follower::updateCommitIndex(std::uint64_t leaderId, const pb::LogAppendReq& request)
        {
            auto commitIndex = engine.getCommitIndex();
            auto lastLogIndex = engine.getStorage()->lastIndex();
            if (request.leader_commit() > commitIndex && commitIndex < lastLogIndex)
            {
                commitIndex = std::min(request.leader_commit(), lastLogIndex);
                engine.setCommitIndex(commitIndex);
            }
        }

        void Follower::logAppendResp(std::uint64_t leaderId, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto lastLogIndex = engine.getStorage()->lastIndex();

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(leaderId);
            raftMsg->set_msg_type(pb::RaftMsg::LOG_APPEND_RESP);

            auto& response = *(raftMsg->mutable_log_append_resp());
            response.set_follower_term(currentTerm);
            response.set_last_log_index(lastLogIndex);
            response.set_success(success);

            outMessages.push_back(std::move(raftMsg));
        }

        void Follower::onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto candidateId = message->from_node_id();
            auto currentTerm = engine.getCurrentTerm();
            auto lastLogIndex = engine.getStorage()->lastIndex();
            auto lastLogTerm = engine.getStorage()->lastTerm();

            CR_ASSERT(message->has_vote_req());
            auto& request = message->vote_req();

            bool success = false;
            if ((request.candidate_term() >= currentTerm && request.candidate_term() > 0)
                && (std::make_tuple(request.last_log_term(), request.last_log_index()) >= std::make_tuple(lastLogTerm, lastLogIndex)))
            {
                if (currentTerm < request.candidate_term())
                {
                    currentTerm = request.candidate_term();
                    setNewerTerm(request.candidate_term());
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
            voteResp(candidateId, request, success, outMessages);
        }

        void Follower::voteResp(std::uint64_t candidateId, const pb::VoteReq& request, bool success, std::vector<RaftMsgPtr>& outMessages)
        {
            RaftMsgPtr raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(candidateId);
            raftMsg->set_msg_type(pb::RaftMsg::VOTE_RESP);

            auto& response = *(raftMsg->mutable_vote_resp());
            response.set_success(success);
            response.set_follower_term(engine.getCurrentTerm());

            outMessages.push_back(std::move(raftMsg));
        }

        void Follower::setNewerTerm(std::uint64_t newerTerm)
        {
            engine.setCurrentTerm(newerTerm);
            engine.setVotedFor(boost::none);
        }
    }
}