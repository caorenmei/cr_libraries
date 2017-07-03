#include <cr/raft/leader.h>

#include <algorithm>

#include <cr/common/assert.h>
#include <cr/raft/raft_engine.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {
        Leader::Leader(RaftEngine& engine)
            : RaftState(engine)
        {}

        Leader::~Leader()
        {}

        void Leader::onEnter(std::shared_ptr<RaftState> prevState)
        {
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            for (auto buddyNodeId : engine.getBuddyNodeIds())
            {
                nextHeartbeatTime_[buddyNodeId] = engine.getNowTime();;
                nextLogIndexs_[buddyNodeId] = lastLogIndex + 1;
                relayLogIndexs_[buddyNodeId] = lastLogIndex;
                matchLogIndexs_[buddyNodeId] = 0;
            }
        }

        void Leader::onLeave()
        {}

        std::uint64_t Leader::update(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto nextUpdateTime = nowTime;
            if (!processOneMessage(nowTime, outMessages))
            {
                updateCommitIndex();
                processLogAppend(nowTime, outMessages);
                nextUpdateTime = checkHeartbeatTimeout(nowTime, outMessages);
            }
            return nextUpdateTime;
        }

        std::uint64_t Leader::updateNextHeartbeatTime(std::uint32_t buddyNodeId, std::uint64_t nowTime)
        {
            auto nextHeartbeatTime = nowTime + engine.getMinElectionTimeout() / 2;
            nextHeartbeatTime_[buddyNodeId] = nextHeartbeatTime;
            return nextHeartbeatTime;
        }

        std::uint64_t Leader::checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto minNextHeartbeatTime = nowTime + engine.getMinElectionTimeout() / 2;
            for (auto buddyNodeId : engine.getBuddyNodeIds())
            {
                auto nextHeartbeatTime = nextHeartbeatTime_[buddyNodeId];
                if (nextHeartbeatTime <= nowTime)
                {
                    logAppendReq(buddyNodeId, outMessages);
                    nextHeartbeatTime = updateNextHeartbeatTime(buddyNodeId, nowTime);
                }
                minNextHeartbeatTime = std::min(minNextHeartbeatTime, nextHeartbeatTime);
            }
            return minNextHeartbeatTime;
        }

        bool Leader::processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
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
                case pb::RaftMsg::LOG_APPEND_RESP:
                    return onLogAppendRespHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::VOTE_REQ:
                    return onVoteReqHandler(nowTime, std::move(message), outMessages);
                }
            }
            return false;
        }

        bool Leader::onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::LogAppendReq request;
            if (request.ParseFromString(message->msg()))
            {
                if (request.leader_term() > engine.getCurrentTerm())
                {
                    engine.getMessageQueue().push_front(std::move(message));
                    setNewerTerm(request.leader_term());
                    return true;
                }
            }
            return false;
        }

        bool Leader::onLogAppendRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::LogAppendResp response;
            if (response.ParseFromString(message->msg()))
            {
                auto currentTerm = engine.getCurrentTerm();
                if (response.follower_term() == currentTerm)
                {
                    auto buddyNodeId = message->from_node_id();
                    auto relayLogIndex = relayLogIndexs_[buddyNodeId];
                    auto matchLogIndex = matchLogIndexs_[buddyNodeId];
                    if (response.success())
                    {
                        matchLogIndexs_[buddyNodeId] = std::max(matchLogIndex, response.last_log_index());
                        relayLogIndexs_[buddyNodeId] = std::max(relayLogIndex, response.last_log_index());
                    }
                    else
                    {
                        nextLogIndexs_[buddyNodeId] = response.last_log_index() + 1;
                        relayLogIndexs_[buddyNodeId] = response.last_log_index();
                    }
                }
                else if (response.follower_term() > currentTerm)
                {
                    setNewerTerm(response.follower_term());
                    return true;
                }
            }
            return false;
        }

        bool Leader::onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            pb::VoteReq request;
            if (request.ParseFromString(message->msg()))
            {
                if (request.candidate_term() > engine.getCurrentTerm())
                {
                    engine.getMessageQueue().push_front(std::move(message));
                    setNewerTerm(request.candidate_term());
                    return true;
                }
            }
            return false;
        }

        void Leader::updateCommitIndex()
        {
            std::vector<std::uint64_t> newCommitIndexs;
            newCommitIndexs.reserve(1 + engine.getBuddyNodeIds().size());
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            newCommitIndexs.push_back(lastLogIndex);
            for (auto&& buddyNodeId : engine.getBuddyNodeIds())
            {
                auto matchLogIndex = matchLogIndexs_[buddyNodeId];
                newCommitIndexs.push_back(matchLogIndex);
            }
            std::sort(newCommitIndexs.begin(), newCommitIndexs.end(), std::greater_equal<std::uint64_t>());
            std::size_t newCommitIndexIndex = static_cast<std::size_t>((1 + engine.getBuddyNodeIds().size()) / 2);
            auto newCommitIndex = newCommitIndexs[newCommitIndexIndex];
            if (engine.getCommitIndex() < newCommitIndex)
            {
                engine.setCommitIndex(newCommitIndex);
            }
        }

        void Leader::processLogAppend(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto logWindowSize = engine.getLogWindowSize();
            auto commitIndex = engine.getCommitIndex();
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            for (auto&& buddyNodeId : engine.getBuddyNodeIds())
            {
                auto nextLogIndex = nextLogIndexs_[buddyNodeId];
                auto relayLogIndex = relayLogIndexs_[buddyNodeId];
                auto machLogIndex = matchLogIndexs_[buddyNodeId];
                if ((nextLogIndex - relayLogIndex <= logWindowSize && nextLogIndex <= lastLogIndex) 
                    || (machLogIndex <= nextLogIndex && machLogIndex < commitIndex))
                {
                    logAppendReq(buddyNodeId, outMessages);
                    updateNextHeartbeatTime(buddyNodeId, nowTime);
                }
            }
        }

        void Leader::logAppendReq(std::uint32_t buddyNodeId, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto commitIndex = engine.getCommitIndex();
            auto nextLogIndex = nextLogIndexs_[buddyNodeId];
            auto prevLogIndex = nextLogIndex - 1;
            auto prevLogTerm = engine.getStorage()->getTermByIndex(prevLogIndex);
            
            pb::LogAppendReq request;
            request.set_leader_term(currentTerm);
            request.set_prev_log_index(prevLogIndex);
            request.set_prev_log_term(prevLogTerm);
            request.set_leader_commit(commitIndex);

            auto lastLogIndex = engine.getStorage()->getLastIndex();
            auto relayLogIndex = relayLogIndexs_[buddyNodeId];
            auto logWindowSize = engine.getLogWindowSize();
            auto maxPacketLenth = engine.getMaxPacketLength();
            logWindowSize = std::max(static_cast<std::uint32_t>(nextLogIndex - relayLogIndex), logWindowSize);
            std::uint32_t logPacketLength = 0;
            while ((nextLogIndex <= lastLogIndex) && (nextLogIndex - relayLogIndex <= logWindowSize) && (logPacketLength < maxPacketLenth))
            {
                auto entries = engine.getStorage()->getEntries(nextLogIndex, nextLogIndex);
                logPacketLength = logPacketLength + entries[0].getValue().size();
                *request.add_entries() = std::move(entries[0].getValue());
                nextLogIndex = nextLogIndex + 1;
            }
            nextLogIndexs_[buddyNodeId] = nextLogIndex;

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(buddyNodeId);
            raftMsg->set_msg_type(pb::RaftMsg::LOG_APPEND_REQ);
            request.SerializeToString(raftMsg->mutable_msg());

            outMessages.push_back(std::move(raftMsg));
        }

        void Leader::setNewerTerm(std::uint32_t newerTerm)
        {
            engine.setCurrentTerm(newerTerm);
            engine.setVotedFor(boost::none);
            engine.setLeaderId(boost::none);
            engine.setNextState(RaftEngine::FOLLOWER);
        }
    }
}