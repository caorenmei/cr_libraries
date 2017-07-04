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
                auto& node = nodes_[nodeId];
                node.nodeId = nodeId;
                node.nextTickTime = engine.getNowTime();;
                node.nextLogIndex = lastLogIndex + 1;
                node.replyLogindex = lastLogIndex;
                node.matchLogIndex = 0;
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

        void Leader::updateNextHeartbeatTime(node& node, std::uint64_t nowTime)
        {
            node.nextTickTime = nowTime + engine.getMinElectionTimeout() / 2;
        }

        std::uint64_t Leader::checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto minNextHeartbeatTime = nowTime + engine.getMinElectionTimeout();
            for (auto&& node : nodes_)
            {
                if (node.second.nextTickTime <= nowTime)
                {
                    logAppendReq(node.second, outMessages);
                    updateNextHeartbeatTime(node.second, nowTime);
                }
                minNextHeartbeatTime = std::min(minNextHeartbeatTime, node.second.nextTickTime);
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
                auto& node = nodes_[nodeId];
                if (response.success())
                {
                    node.matchLogIndex = std::max(node.matchLogIndex, response.last_log_index());
                    node.replyLogindex = std::max(node.replyLogindex, response.last_log_index());
                }
                else
                {
                    node.nextLogIndex = response.last_log_index() + 1;
                    node.replyLogindex = response.last_log_index();
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
            for (auto&& node : nodes_)
            {
                newCommitIndexs.push_back(node.second.matchLogIndex);
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
            for (auto&& node : nodes_)
            {
                if ((node.second.nextLogIndex - node.second.replyLogindex <= logWindowSize && node.second.nextLogIndex <= lastLogIndex)
                    || (node.second.matchLogIndex <= node.second.nextLogIndex && node.second.matchLogIndex < commitIndex))
                {
                    logAppendReq(node.second, outMessages);
                    updateNextHeartbeatTime(node.second, nowTime);
                }
            }
        }

        void Leader::logAppendReq(node& node, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto commitIndex = engine.getCommitIndex();
            auto prevLogIndex = node.nextLogIndex - 1;
            auto prevLogTerm = engine.getStorage()->getTermByIndex(prevLogIndex);

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(node.nodeId);
            raftMsg->set_msg_type(pb::RaftMsg::LOG_APPEND_REQ);

            auto& request = *(raftMsg->mutable_log_append_req());
            request.set_leader_term(currentTerm);
            request.set_prev_log_index(prevLogIndex);
            request.set_prev_log_term(prevLogTerm);
            request.set_leader_commit(commitIndex);

            auto lastLogIndex = engine.getStorage()->getLastIndex();
            auto logWindowSize = engine.getLogWindowSize();
            auto maxPacketLenth = engine.getMaxPacketLength();
            logWindowSize = std::max<std::uint32_t>(node.nextLogIndex - node.replyLogindex, logWindowSize);
            while ((node.nextLogIndex <= lastLogIndex) && (node.nextLogIndex - node.replyLogindex <= logWindowSize) && (request.ByteSize() < maxPacketLenth))
            {
                auto entries = engine.getStorage()->getEntries(node.nextLogIndex, node.nextLogIndex);
                *request.add_entries() = std::move(entries[0].getValue());
                node.nextLogIndex = node.nextLogIndex + 1;
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