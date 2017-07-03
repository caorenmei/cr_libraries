#include <cr/raft/candidate.h>

#include <limits>
#include <random>
#include <tuple>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {

        Candidate::Candidate(RaftEngine& engine)
            : RaftState(engine),
            nextElectionTime_(0)
        {}

        Candidate::~Candidate()
        {}

        void Candidate::onEnter(std::shared_ptr<RaftState> prevState)
        {
            nextElectionTime_ = engine.getNowTime();
        }

        void Candidate::onLeave()
        {}

        std::uint64_t Candidate::update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto nextUpdateTime = nowTime;
            if (!checkElectionTimeout(nowTime, outMessages))
            {
                if (!processOneMessage(nowTime, outMessages))
                {
                    if (engine.getMessageQueue().empty())
                    {
                        nowTime = nextElectionTime_;
                    }
                }
            }
            return nextUpdateTime;
        }

        void Candidate::updateNextElectionTime(std::uint64_t nowTime)
        {
            nextElectionTime_ = nowTime + engine.randomElectionTimeout();
        }

        bool Candidate::checkElectionTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            if (nextElectionTime_ <= nowTime)
            {
                updateNextElectionTime(nowTime);
                engine.setCurrentTerm(engine.getCurrentTerm() + 1);
                grantNodeIds_.clear();
                grantNodeIds_.insert(engine.getNodeId());
                processVoteReq(nowTime, outMessages);
                return checkVoteGranted(nowTime);
            }
            return false;
        }

        void Candidate::processVoteReq(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            auto lastLogTerm = engine.getStorage()->getLastTerm();
            for (auto buddyNodeId : engine.getBuddyNodeIds())
            {
                pb::VoteReq request;
                request.set_candidate_term(currentTerm);
                request.set_last_log_index(lastLogIndex);
                request.set_last_log_term(lastLogTerm);

                auto raftMsg = std::make_shared<pb::RaftMsg>();
                raftMsg->set_from_node_id(engine.getNodeId());
                raftMsg->set_dest_node_id(buddyNodeId);
                raftMsg->set_msg_type(pb::RaftMsg::VOTE_REQ);
                request.SerializeToString(raftMsg->mutable_msg());

                outMessages.push_back(std::move(raftMsg));
            }
        }

        bool Candidate::processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
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
                    return onLogAppendReqHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::VOTE_REQ:
                    return onVoteReqHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::VOTE_RESP:
                    return onVoteRespHandler(nowTime, std::move(message), outMessages);
                }
            }
            return false;
        }

        bool Candidate::onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::LogAppendReq request;
            if (request.ParseFromString(message->msg()))
            {
                if (engine.getCurrentTerm() <= request.leader_term())
                {
                    engine.getMessageQueue().push_front(std::move(message));
                    engine.setNextState(RaftEngine::FOLLOWER);
                    engine.setVotedFor(boost::none);
                    return true;
                }
            }
            return false;
        }

        bool Candidate::onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::VoteReq request;
            if (request.ParseFromString(message->msg()))
            {
                if (engine.getCurrentTerm() < request.candidate_term())
                {
                    engine.getMessageQueue().push_front(std::move(message));
                    setNewerTerm(request.candidate_term());
                    return true;
                }
            }
            return false;
        }

        bool Candidate::onVoteRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            
            pb::VoteResp response;
            if (response.ParseFromString(message->msg()))
            {
                auto followerId = message->from_node_id();
                auto currentTerm = engine.getCurrentTerm();
                if (currentTerm == response.candidate_term() && response.success())
                {
                    grantNodeIds_.insert(followerId);
                    if (checkVoteGranted(nowTime))
                    {
                        return true;
                    }
                }
                else if (currentTerm < response.follower_term())
                {
                    currentTerm = response.follower_term();
                    setNewerTerm(currentTerm);
                    return true;
                }
            }
            return false;
        }

        bool Candidate::checkVoteGranted(std::uint64_t nowTime)
        {
            if (grantNodeIds_.size() > (1 + engine.getBuddyNodeIds().size()) / 2)
            {
                engine.setNextState(RaftEngine::LEADER);
                engine.setLeaderId(engine.getNodeId());
                return true;
            }
            return false;
        }

        void Candidate::setNewerTerm(std::uint32_t newerTerm)
        {
            engine.setCurrentTerm(newerTerm);
            engine.setVotedFor(boost::none);
            engine.setNextState(RaftEngine::FOLLOWER);
        }
    }
}