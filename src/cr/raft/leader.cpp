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
            for (auto nodeId : engine.getBuddyNodeIds())
            {
                auto& BuddyNode = nodes_[nodeId];
                BuddyNode.nodeId = nodeId;
                BuddyNode.nextUpdateTime = engine.getNowTime();;
                BuddyNode.nextLogIndex = lastLogIndex + 1;
                BuddyNode.replyLogindex = lastLogIndex;
                BuddyNode.matchLogIndex = 0;
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

        void Leader::updateNextUpdateTime(BuddyNode& BuddyNode, std::uint64_t nowTime)
        {
            BuddyNode.nextUpdateTime = nowTime + engine.getMinElectionTimeout() / 2;
        }

        std::uint64_t Leader::checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto minNextUpdateTime = nowTime + engine.getMinElectionTimeout();
            for (auto&& BuddyNode : nodes_)
            {
                if (BuddyNode.second.nextUpdateTime <= nowTime)
                {
                    logAppendReq(BuddyNode.second, outMessages);
                    updateNextUpdateTime(BuddyNode.second, nowTime);
                }
                minNextUpdateTime = std::min(minNextUpdateTime, BuddyNode.second.nextUpdateTime);
            }
            return minNextUpdateTime;
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
            CR_ASSERT(message->has_log_append_req());
            auto& request = message->log_append_req();
            if (request.leader_term() > engine.getCurrentTerm())
            {
                engine.getMessageQueue().push_front(std::move(message));
                setNewerTerm(request.leader_term());
                return true;
            }
            return false;
        }

        bool Leader::onLogAppendRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();

            CR_ASSERT(message->has_log_append_resp());
            auto& response = message->log_append_resp();

            if (response.follower_term() == currentTerm)
            {
                auto nodeId = message->from_node_id();
                auto& BuddyNode = nodes_[nodeId];
                if (response.success())
                {
                    BuddyNode.matchLogIndex = std::max(BuddyNode.matchLogIndex, response.last_log_index());
                    BuddyNode.replyLogindex = std::max(BuddyNode.replyLogindex, response.last_log_index());
                }
                else
                {
                    BuddyNode.nextLogIndex = response.last_log_index() + 1;
                    BuddyNode.replyLogindex = response.last_log_index();
                }
            }
            else if (response.follower_term() > currentTerm)
            {
                setNewerTerm(response.follower_term());
                return true;
            }
            return false;
        }

        bool Leader::onVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            CR_ASSERT(message->has_vote_req());
            auto& request = message->vote_req();
            if (request.candidate_term() > engine.getCurrentTerm())
            {
                engine.getMessageQueue().push_front(std::move(message));
                setNewerTerm(request.candidate_term());
                return true;
            }
            return false;
        }

        void Leader::updateCommitIndex()
        {
            std::vector<std::uint64_t> newCommitIndexs;
            newCommitIndexs.reserve(1 + nodes_.size());
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            newCommitIndexs.push_back(lastLogIndex);
            for (auto&& BuddyNode : nodes_)
            {
                newCommitIndexs.push_back(BuddyNode.second.matchLogIndex);
            }
            std::sort(newCommitIndexs.begin(), newCommitIndexs.end(), std::greater_equal<std::uint64_t>());
            std::size_t newCommitIndexIndex = static_cast<std::size_t>((1 + nodes_.size()) / 2);
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
            for (auto&& BuddyNode : nodes_)
            {
                if ((BuddyNode.second.nextLogIndex - BuddyNode.second.replyLogindex <= logWindowSize && BuddyNode.second.nextLogIndex <= lastLogIndex)
                    || (BuddyNode.second.matchLogIndex <= BuddyNode.second.nextLogIndex && BuddyNode.second.matchLogIndex < commitIndex))
                {
                    logAppendReq(BuddyNode.second, outMessages);
                    updateNextUpdateTime(BuddyNode.second, nowTime);
                }
            }
        }

        void Leader::logAppendReq(BuddyNode& BuddyNode, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto commitIndex = engine.getCommitIndex();
            auto prevLogIndex = BuddyNode.nextLogIndex - 1;
            auto prevLogTerm = engine.getStorage()->getTermByIndex(prevLogIndex);

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(BuddyNode.nodeId);
            raftMsg->set_msg_type(pb::RaftMsg::LOG_APPEND_REQ);

            auto& request = *(raftMsg->mutable_log_append_req());
            request.set_leader_term(currentTerm);
            request.set_prev_log_index(prevLogIndex);
            request.set_prev_log_term(prevLogTerm);
            request.set_leader_commit(commitIndex);

            auto lastLogIndex = engine.getStorage()->getLastIndex();
            auto logWindowSize = engine.getLogWindowSize();
            auto maxPacketLenth = engine.getMaxPacketLength();
            logWindowSize = std::max<std::uint32_t>(BuddyNode.nextLogIndex - BuddyNode.replyLogindex, logWindowSize);
            while ((BuddyNode.nextLogIndex <= lastLogIndex) && (BuddyNode.nextLogIndex - BuddyNode.replyLogindex <= logWindowSize) && (request.ByteSize() < maxPacketLenth))
            {
                auto entries = engine.getStorage()->getEntries(BuddyNode.nextLogIndex, BuddyNode.nextLogIndex);
                *request.add_entries() = std::move(entries[0].getValue());
                BuddyNode.nextLogIndex = BuddyNode.nextLogIndex + 1;
            }

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