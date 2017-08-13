#include "leader.h"

#include <algorithm>

#include <cr/common/assert.h>

#include "raft.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {
        Leader::Leader(Raft& raft)
            : RaftState(raft)
        {}

        Leader::~Leader()
        {}

        int Leader::getState() const
        {
            return Raft::LEADER;
        }

        void Leader::onEnter(std::shared_ptr<RaftState> prevState)
        {
            auto lastLogIndex = raft.getStorage()->getLastIndex();
            for (auto nodeId : raft.getBuddyNodeIds())
            {
                nodes_.emplace_back();
                auto& node = nodes_.back();
                node.nodeId = nodeId;
                node.nextUpdateTime = raft.getNowTime();;
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
            // 如果没有状态转换
            if (!processOneMessage(nowTime, outMessages))
            {
                // 发送日志
                nextUpdateTime = processAppendEntriesReq(nowTime, outMessages);
            }
            return nextUpdateTime;
        }

        bool Leader::processOneMessage(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            if (!raft.getMessageQueue().empty())
            {
                auto message = std::move(raft.getMessageQueue().front());
                raft.getMessageQueue().pop_front();
                switch (message->msg_type())
                {
                case pb::RaftMsg::APPEND_ENTRIES_REQ:
                    return onAppendEntriesReqHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::APPEND_ENTRIES_RESP:
                    return onAppendEntriesRespHandler(nowTime, std::move(message), outMessages);
                case pb::RaftMsg::REQUEST_VOTE_REQ:
                    return onRequestVoteReqHandler(nowTime, std::move(message), outMessages);
                }
            }
            return false;
        }

        bool Leader::onAppendEntriesReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto& request = message->append_entries_req();
            auto currentTerm = raft.getCurrentTerm();
            // 如果是新的领导者,则转换到跟随者
            if (request.leader_term() > currentTerm)
            {
                raft.getMessageQueue().push_front(std::move(message));
                currentTerm = request.leader_term();
                raft.setNextState(Raft::FOLLOWER);
                raft.setCurrentTerm(currentTerm);
                raft.setLeaderId(boost::none);
                return true;
            }
            return false;
        }

        bool Leader::onAppendEntriesRespHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto& response = message->append_entries_resp();
            auto currentTerm = raft.getCurrentTerm();
            // 如果是本任期的消息
            if (response.follower_term() == currentTerm)
            {
                auto nodeId = message->from_node_id();
                auto nodeIter = std::find_if(nodes_.begin(), nodes_.end(), [&](auto& node) { return node.nodeId == nodeId; });
                CR_ASSERT(nodeIter != nodes_.end());
                auto& node = *nodeIter;
                auto lastLogIndex = raft.getStorage()->getLastIndex();
                auto nodeLastLogIndex = std::min(lastLogIndex, response.last_log_index());
                //日志匹配成功,更新伙伴节点信息
                if (response.success())
                {
                    node.replyLogIndex = std::max(node.replyLogIndex, nodeLastLogIndex);
                    node.matchLogIndex = std::max(node.matchLogIndex, node.replyLogIndex);
                }
                // 匹配失败，减小节点重试
                else
                {
                    node.nextLogIndex = nodeLastLogIndex + 1;
                    node.replyLogIndex = nodeLastLogIndex;
                    node.matchLogIndex = std::min(node.matchLogIndex, node.replyLogIndex);
                }
            }
            // 如果对方任期更大，则转换到跟随者
            else if (response.follower_term() > currentTerm)
            {
                currentTerm = response.follower_term();
                raft.setNextState(Raft::FOLLOWER);
                raft.setCurrentTerm(currentTerm);
                raft.setLeaderId(boost::none);
                return true;
            }
            return false;
        }

        bool Leader::onRequestVoteReqHandler(std::uint64_t nowTime, RaftMsgPtr message, std::vector<RaftMsgPtr>& outMessages)
        {
            auto& request = message->request_vote_req();
            auto currentTerm = raft.getCurrentTerm();
            // 如果候选者任期更新，则转换为跟随者
            if (request.candidate_term() > raft.getCurrentTerm())
            {
                raft.getMessageQueue().push_front(std::move(message));
                currentTerm = request.candidate_term();
                raft.setNextState(Raft::FOLLOWER);
                raft.setCurrentTerm(currentTerm);
                raft.setLeaderId(boost::none);
                return true;
            }
            return false;
        }

        std::uint64_t Leader::calNewCommitIndex()
        {
            matchLogIndexs_.clear();
            // 对所有已匹配日志排序
            auto lastLogIndex = raft.getStorage()->getLastIndex();
            matchLogIndexs_.push_back(lastLogIndex);
            for (auto&& node : nodes_)
            {
                matchLogIndexs_.push_back(node.matchLogIndex);
            }
            std::sort(matchLogIndexs_.begin(), matchLogIndexs_.end(), std::greater<std::uint64_t>());
            // 大部分节点都已提交N，则N为已提交日志索引
            auto commitIndexIndex = (1 + nodes_.size()) / 2;
            return std::min(matchLogIndexs_[commitIndexIndex], lastLogIndex);
        }

        std::uint64_t Leader::processAppendEntriesReq(std::uint64_t nowTime, std::vector<RaftMsgPtr>& outMessages)
        {
            auto commitIndex = raft.getCommitIndex();
            auto newCommitIndex = calNewCommitIndex();
            // 更新已提交日志索引
            bool updateCommitIndex = false;
            if (commitIndex < newCommitIndex)
            {
                commitIndex = newCommitIndex;
                raft.setCommitIndex(commitIndex);
                updateCommitIndex = true;
            }
            std::uint64_t nextUpdateTime = nowTime + raft.getHeatbeatTimeout();
            // 更新节点
            auto lastLogIndex = raft.getStorage()->getLastIndex();
            auto maxWaitEntriesNum = raft.getMaxWaitEntriesNum();
            for (auto&& node : nodes_)
            {
                // 需要发送心跳或需要复制日志
                if (((node.nextLogIndex - node.replyLogIndex <= maxWaitEntriesNum) 
                    && (updateCommitIndex || (node.nextLogIndex <= lastLogIndex))) || (node.nextUpdateTime <= nowTime))
                {
                    // 传送日志
                    appendEntriesReq(node, outMessages);
                    // 更新心跳时间
                    node.nextUpdateTime = nowTime + raft.getHeatbeatTimeout();
                }
                nextUpdateTime = std::min(nextUpdateTime, node.nextUpdateTime);
            }
            return nextUpdateTime;
        }

        void Leader::appendEntriesReq(BuddyNode& node, std::vector<RaftMsgPtr>& outMessages)
        {
            auto raftMsg = std::make_shared<pb::RaftMsg>();
            raftMsg->set_from_node_id(raft.getNodeId());
            raftMsg->set_dest_node_id(node.nodeId);
            raftMsg->set_msg_type(pb::RaftMsg::APPEND_ENTRIES_REQ);

            auto currentTerm = raft.getCurrentTerm();
            auto commitIndex = raft.getCommitIndex();
            auto prevLogIndex = node.nextLogIndex - 1;
            auto prevLogTerm = prevLogIndex != 0 ? raft.getStorage()->getTermByIndex(prevLogIndex) : 0;

            auto& request = *(raftMsg->mutable_append_entries_req());
            request.set_leader_term(currentTerm);
            request.set_prev_log_index(prevLogIndex);
            request.set_prev_log_term(prevLogTerm);
            request.set_leader_commit(commitIndex);

            auto lastLogIndex = raft.getStorage()->getLastIndex();
            if (node.nextLogIndex <= lastLogIndex)
            {
                auto stopLogIndex = std::min(lastLogIndex, node.nextLogIndex + raft.getMaxPacketEntriesNum() - 1);
                stopLogIndex = std::min(stopLogIndex, node.replyLogIndex + raft.getMaxWaitEntriesNum());
                stopLogIndex = std::max(stopLogIndex, node.nextLogIndex);
                auto entries = raft.getStorage()->getEntries(node.nextLogIndex, stopLogIndex, raft.getMaxPacketLength());
                for (auto& entry : entries)
                {
                    *request.add_entries() = std::move(entry);
                }
                node.nextLogIndex = node.nextLogIndex + entries.size();
            }

            outMessages.push_back(std::move(raftMsg));
        }
    }
}