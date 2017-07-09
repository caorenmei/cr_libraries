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

        int Leader::getState() const
        {
            return RaftEngine::LEADER;
        }

        void Leader::onEnter(std::shared_ptr<RaftState> prevState)
        {
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            for (auto nodeId : engine.getBuddyNodeIds())
            {
                auto& node = nodes_[nodeId];
                node.nodeId = nodeId;
                node.nextUpdateTime = engine.getNowTime();;
                node.nextLogIndex = lastLogIndex + 1;
                node.replyLogIndex = lastLogIndex;
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
                bool newCommitIndex = updateCommitIndex();
                processLogAppend(nowTime, newCommitIndex, outMessages);
                nextUpdateTime = checkHeartbeatTimeout(nowTime, outMessages);
            }
            return nextUpdateTime;
        }

        void Leader::updateNextUpdateTime(BuddyNode& node, std::uint64_t nowTime)
        {
            node.nextUpdateTime = nowTime + engine.getHeatbeatTimeout();
        }

        std::uint64_t Leader::checkHeartbeatTimeout(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto minNextUpdateTime = nowTime + engine.getHeatbeatTimeout();
            for (auto&& node : nodes_)
            {
                if (node.second.nextUpdateTime <= nowTime)
                {
                    logAppendReq(node.second, outMessages);
                    updateNextUpdateTime(node.second, nowTime);
                }
                minNextUpdateTime = std::min(minNextUpdateTime, node.second.nextUpdateTime);
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
                case pb::RaftMsg::APPEND_ENTRIES_REQ:
                    return onLogAppendReqHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::APPEND_ENTRIES_RESP:
                    return onLogAppendRespHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::REQUEST_VOTE_REQ:
                    return onVoteReqHandler(nowTime, std::move(message), outMessages);
                }
            }
            return false;
        }

        bool Leader::onLogAppendReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            CR_ASSERT(message->has_append_entries_req());
            auto& request = message->append_entries_req();
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

            CR_ASSERT(message->has_append_entries_resp());
            auto& response = message->append_entries_resp();

            if (response.follower_term() == currentTerm)
            {
                auto nodeId = message->from_node_id();
                auto& node = nodes_[nodeId];
                if (response.success())
                {
                    node.matchLogIndex = std::max(node.matchLogIndex, response.last_log_index());
                    node.replyLogIndex = std::max(node.replyLogIndex, response.last_log_index());
                }
                else
                {
                    node.nextLogIndex = response.last_log_index() + 1;
                    node.replyLogIndex = response.last_log_index();
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
            CR_ASSERT(message->has_request_vote_req());
            auto& request = message->request_vote_req();
            if (request.candidate_term() > engine.getCurrentTerm())
            {
                engine.getMessageQueue().push_front(std::move(message));
                setNewerTerm(request.candidate_term());
                return true;
            }
            return false;
        }

        bool Leader::updateCommitIndex()
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
                return true;
            }
            return false;
        }

        void Leader::processLogAppend(std::uint64_t nowTime, bool newerCommitIndex, std::vector<RaftMsgPtr>& outMessages)
        {
            auto logWindowSize = engine.getMaxEntriesNum();
            auto commitIndex = engine.getCommitIndex();
            auto lastLogIndex = engine.getStorage()->getLastIndex();
            for (auto&& node : nodes_)
            {
                auto nodeWindowSize = std::max(node.second.nextLogIndex - node.second.replyLogIndex, logWindowSize);
                if ((node.second.nextLogIndex - node.second.replyLogIndex <= nodeWindowSize && node.second.nextLogIndex <= lastLogIndex)
                    || ((node.second.matchLogIndex <= node.second.nextLogIndex) && (node.second.matchLogIndex < commitIndex || newerCommitIndex)))
                {
                    logAppendReq(node.second, outMessages);
                    updateNextUpdateTime(node.second, nowTime);
                }
            }
        }

        void Leader::logAppendReq(BuddyNode& node, std::vector<RaftMsgPtr>& outMessages)
        {
            auto currentTerm = engine.getCurrentTerm();
            auto commitIndex = engine.getCommitIndex();
            auto prevLogIndex = node.nextLogIndex - 1;
            auto prevLogTerm = prevLogIndex != 0 ? engine.getStorage()->getTermByIndex(prevLogIndex) : 0;

            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(engine.getNodeId());
            raftMsg->set_dest_node_id(node.nodeId);
            raftMsg->set_msg_type(pb::RaftMsg::APPEND_ENTRIES_REQ);

            auto& request = *(raftMsg->mutable_append_entries_req());
            request.set_leader_term(currentTerm);
            request.set_prev_log_index(prevLogIndex);
            request.set_prev_log_term(prevLogTerm);
            request.set_leader_commit(commitIndex);

            auto lastLogIndex = engine.getStorage()->getLastIndex();
            auto logWindowSize = engine.getMaxEntriesNum();
            auto maxPacketLenth = engine.getMaxPacketLength();
            logWindowSize = std::max<std::uint64_t>(node.nextLogIndex - node.replyLogIndex, logWindowSize);
            maxPacketLenth = std::max<std::uint64_t>(request.ByteSize() + 1, maxPacketLenth);
            while ((node.nextLogIndex <= lastLogIndex) && (node.nextLogIndex - node.replyLogIndex <= logWindowSize) && (request.ByteSize() < maxPacketLenth))
            {
                auto getEntries = engine.getStorage()->getEntries(node.nextLogIndex, node.nextLogIndex);
                *request.add_entries() = std::move(getEntries[0].getValue());
                node.nextLogIndex = node.nextLogIndex + 1;
            }

            outMessages.push_back(std::move(raftMsg));
        }

        void Leader::setNewerTerm(std::uint64_t newerTerm)
        {
            engine.setCurrentTerm(newerTerm);
            engine.setVotedFor(boost::none);
            engine.setLeaderId(boost::none);
            engine.setNextState(RaftEngine::FOLLOWER);
        }
    }
}