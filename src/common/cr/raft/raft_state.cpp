#include "raft_state.h"

#include "buddy_node.h"
#include "raft.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {
        // 跟随者状态
        constexpr std::size_t FOLLOWER_STATE = 0;
        // 候选者状态
        constexpr std::size_t CANDIDATE_STATE = 1;
        // 领导者状态
        constexpr std::size_t LEADER_STATE = 2;
        

        RaftState_::RaftState_(Raft& raft)
            : raft_(raft),
            nowTime_(0)
        {}

        RaftState_::~RaftState_()
        {}

        void RaftState_::on_entry(const StartUpEvent&, RaftState& fsm)
        {
            states_[FOLLOWER_STATE] = fsm.get_state_by_id(FOLLOWER_STATE);
            states_[CANDIDATE_STATE] = fsm.get_state_by_id(CANDIDATE_STATE);
            states_[LEADER_STATE] = fsm.get_state_by_id(LEADER_STATE);
        }

        void RaftState_::on_exit(const StartUpEvent&, RaftState&)
        {}

        std::uint64_t RaftState_::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto self = static_cast<RaftState*>(this);
            auto curState = states_[self->current_state()[0]];
            return curState->update(messages);
        }

        Raft& RaftState_::getRaft()
        {
            return raft_;
        }

        const Raft& RaftState_::getRaft() const
        {
            return raft_;
        }

        void RaftState_::setNowTime(std::uint64_t nowTime)
        {
            nowTime_ = nowTime;
        }

        std::uint64_t RaftState_::getNowTime() const
        {
            return nowTime_;
        }

        bool RaftState_::isFollower() const
        {
            return static_cast<const RaftState*>(this)->current_state()[0] == FOLLOWER_STATE;
        }

        bool RaftState_::isCandidate() const
        {
            return static_cast<const RaftState*>(this)->current_state()[0] == CANDIDATE_STATE;
        }

        bool RaftState_::isLeader() const
        {
            return static_cast<const RaftState*>(this)->current_state()[0] == LEADER_STATE;
        }

        std::deque<std::shared_ptr<pb::RaftMsg>>& RaftState_::getMessages()
        {
            return messages_;
        }

        const std::deque<std::shared_ptr<pb::RaftMsg>>& RaftState_::getMessages() const
        {
            return messages_;
        }

        std::uint64_t RaftState_::randElectionTimeout() const
        {
            auto& options = raft_.getOptions();
            auto& random = raft_.getRandom();
            std::uniform_int_distribution<std::uint64_t> distribution(options.getMinElectionTimeout(), options.getMaxElectionTimeout());
            return distribution(random);
        }

        FollowerState::FollowerState()
            : state_(nullptr),
            electionTime_(0)
        {}

        FollowerState::~FollowerState()
        {}

        void FollowerState::on_entry(const StartUpEvent&, RaftState&)
        {
            electionTime_ = state_->getNowTime() + state_->randElectionTimeout();
        }

        void FollowerState::on_entry(const DiscoversEvent&, RaftState&)
        {
            electionTime_ = state_->getNowTime() + state_->randElectionTimeout();
        }

        void FollowerState::set_sm_ptr(RaftState* sm)
        {
            state_ = sm;
        }

        std::uint64_t FollowerState::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto nowTime = state_->getNowTime();
            // 先检查超时
            if (nowTime >= electionTime_)
            {
                state_->process_event(ElectionTimeoutEvent());
                return nowTime;
            }
            // 处理消息
            auto& messageQueue = state_->getMessages();
            while (!messageQueue.empty())
            {
                auto message = messageQueue.front();
                messageQueue.pop_front();
                // 追加日志
                if (message->has_append_entries_req())
                {
                    handleAppendEntriesReq(message, messages);
                }
                else if (message->has_append_entries_resp())
                {

                }
                // 投票请求
                else if (message->has_request_vote_req())
                {
                    handleRequestVoteReq(message, messages);
                }
                else if (message->has_request_vote_resp())
                {
                    
                }
            }
            return electionTime_;
        }

        void FollowerState::handleAppendEntriesReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& appendEntriesReq = request->append_entries_req();
            // 任期过期
            if (appendEntriesReq.leader_term() < raft.getCurrentTerm())
            {
                sendAppendEntriesResp(request, false, messages);
                return;
            }
            // 更新任期
            if (appendEntriesReq.leader_term() > raft.getCurrentTerm())
            {
                raft.setCurrentTerm(appendEntriesReq.leader_term());
                raft.setVotedFor(boost::none);
            }
            // 更新leader
            auto leaderId = raft.getLeaderId();
            if (!leaderId.is_initialized() || leaderId.get() != request->from_node_id())
            {
                raft.setLeaderId(request->from_node_id());
            }
            // 超时时间
            electionTime_ = state_->getNowTime() + state_->randElectionTimeout();
            auto lastLogIndex = storage->getLastIndex();
            // 缺少日志
            if (lastLogIndex < appendEntriesReq.prev_log_index())
            {
                sendAppendEntriesResp(request, false, messages);
                return;
            }
            // 匹配日志
            auto prevLogTerm = storage->getTermByIndex(appendEntriesReq.prev_log_index());
            if (appendEntriesReq.prev_log_term() != prevLogTerm)
            {
                sendAppendEntriesResp(request, false, messages);
                return;
            }
            // 日志条目和新的产生冲突，删除这一条和之后所有的
            auto appendLogIndex = appendEntriesReq.prev_log_index() + 1;
            int entriesIndex = 0;
            while (appendLogIndex <= lastLogIndex && entriesIndex < appendEntriesReq.entries_size())
            {
                auto& appendEntry = appendEntriesReq.entries(entriesIndex);
                auto appendLogTerm = storage->getTermByIndex(appendLogIndex);
                if (appendLogTerm != appendEntry.term())
                {
                    storage->remove(appendLogIndex);
                    break;
                }
                appendLogIndex = appendLogIndex + 1;
                entriesIndex = entriesIndex + 1;
            }
            // 追加日志
            std::vector<pb::Entry> appendEntries;
            for (; entriesIndex != appendEntriesReq.entries_size(); ++entriesIndex)
            {
                appendEntries.push_back(appendEntriesReq.entries(entriesIndex));
            }
            storage->append(appendEntries);
            // 更新提交索引
            if (appendEntriesReq.leader_commit() > raft.getCommitIndex())
            {
                auto commitIndex = std::min(storage->getLastIndex(), appendEntriesReq.leader_commit());
                raft.setCommitIndex(commitIndex);
            }
            // 应答
            sendAppendEntriesResp(request, true, messages);
        }

        void FollowerState::sendAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& appendEntriesReq = request->append_entries_req();
            auto appendLogIndex = appendEntriesReq.prev_log_index() + appendEntriesReq.entries_size();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(request->from_node_id());
            // 应答信息
            auto response = message->mutable_append_entries_resp();
            response->set_follower_term(raft.getCurrentTerm());
            response->set_leader_term(appendEntriesReq.leader_term());
            response->set_prev_log_index(appendEntriesReq.prev_log_index());
            response->set_append_log_index(appendLogIndex);
            response->set_last_log_index(storage->getLastIndex());
            response->set_success(success);
            // 发送
            messages.push_back(message);
        }

        void FollowerState::handleRequestVoteReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& requestVoteReq = request->request_vote_req();
            // 判断任期
            if (requestVoteReq.candidate_term() < raft.getCurrentTerm())
            {
                sendRequestVoteResp(request, false, messages);
                return;
            }
            // 更新任期
            if (requestVoteReq.candidate_term() > raft.getCurrentTerm())
            {
                raft.setCurrentTerm(requestVoteReq.candidate_term());
                raft.setVotedFor(boost::none);
            }
            // 判断是否已投票
            auto votedFor = raft.getVotedFor();
            if (votedFor.is_initialized() && votedFor.get() != request->from_node_id())
            {
                sendRequestVoteResp(request, false, messages);
                return;
            }
            // 候选人的日志没有自己新
            auto lastLogIndex = storage->getLastIndex();
            auto lastLogTerm = storage->getLastTerm();
            if (std::make_tuple(lastLogTerm, lastLogIndex) > std::make_tuple(requestVoteReq.last_log_term(), requestVoteReq.last_log_index()))
            {
                sendRequestVoteResp(request, false, messages);
                return;
            }
            // 投票
            raft.setVotedFor(request->from_node_id());
            sendRequestVoteResp(request, true, messages);
        }

        void FollowerState::sendRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& requestVoteReq = request->request_vote_req();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(request->from_node_id());
            // 应答信息
            auto response = message->mutable_request_vote_resp();
            response->set_follower_term(raft.getCurrentTerm());
            response->set_candidate_term(requestVoteReq.candidate_term());
            response->set_success(success);
            // 发送
            messages.push_back(message);
        }

        CandidateState::CandidateState()
            : state_(nullptr),
            electionTime_(0)
        {}

        CandidateState::~CandidateState()
        {}

        void CandidateState::on_entry(const ElectionTimeoutEvent&, RaftState&)
        {
            // 清除领导者属性
            auto& raft = state_->getRaft();
            raft.setVotedFor(boost::none);
            raft.setLeaderId(boost::none);
            // 更新选举超时
            electionTime_ = state_->getNowTime();
        }

        void CandidateState::set_sm_ptr(RaftState* sm)
        {
            state_ = sm;
        }

        std::uint64_t CandidateState::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto nowTime = state_->getNowTime();
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            // 选举超时
            if (electionTime_ <= nowTime)
            {
                // 自增任期号
                auto currentTerm = raft.getCurrentTerm();
                raft.setCurrentTerm(currentTerm + 1);
                //给自己一票
                votes_.clear();
                raft.setVotedFor(options.getNodeId());
                votes_.insert(options.getNodeId());
                // 发送投票请求
                sendRequestVoteReq(messages);
                // 选举超时时间
                electionTime_ = nowTime + state_->randElectionTimeout();
            }
            // 处理消息
            auto& messageQueue = state_->getMessages();
            while (!messageQueue.empty())
            {
                auto message = messageQueue.front();
                messageQueue.pop_front();
                // 追加日志
                if (message->has_append_entries_req())
                {
                    if (handleAppendEntriesReq(message, messages))
                    {
                        return nowTime;
                    }
                }
                else if (message->has_append_entries_resp())
                {

                }
                // 投票请求
                else if (message->has_request_vote_req())
                {
                    if (handleRequestVoteReq(message, messages))
                    {
                        return nowTime;
                    }
                }
                // 投票请求回复
                else if (message->has_request_vote_resp())
                {
                    if (handleRequestVoteResp(message, messages))
                    {
                        return nowTime;
                    }
                }
            }
            // 检查投票, 超过半数投票，成为跟随着
            if (votes_.size() > (1 + raft.getBuddyNodeIds().size()) / 2)
            {
                state_->process_event(MajorityVotesEvent());
                return nowTime;
            }
            // 下一次超时时间
            return electionTime_;
        }

        bool CandidateState::handleAppendEntriesReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& appendEntriesReq = request->append_entries_req();
            // 任期更新，转换为跟随者
            if (appendEntriesReq.leader_term() >= raft.getCurrentTerm())
            {
                auto& messageQueue = state_->getMessages();
                messageQueue.push_front(request);
                state_->process_event(DiscoversEvent());
                return true;
            }
            // 否则，日志追加失败
            else
            {
                sendAppendEntriesResp(request, false, messages);
                return false;
            }
        }

        void CandidateState::sendAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& appendEntriesReq = request->append_entries_req();
            auto appendLogIndex = appendEntriesReq.prev_log_index() + appendEntriesReq.entries_size();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(request->from_node_id());
            // 应答信息
            auto response = message->mutable_append_entries_resp();
            response->set_follower_term(raft.getCurrentTerm());
            response->set_leader_term(appendEntriesReq.leader_term());
            response->set_prev_log_index(appendEntriesReq.prev_log_index());
            response->set_append_log_index(appendLogIndex);
            response->set_last_log_index(storage->getLastIndex());
            response->set_success(success);
            // 发送
            messages.push_back(message);
        }

        bool CandidateState::handleRequestVoteReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& requestVoteReq = request->request_vote_req();
            // 任期更新，转换为跟随者
            if (requestVoteReq.candidate_term() > raft.getCurrentTerm())
            {
                auto& messageQueue = state_->getMessages();
                messageQueue.push_front(request);
                state_->process_event(DiscoversEvent());
                return true;
            }
            // 否则，投票失败
            else
            {
                sendRequestVoteResp(request, false, messages);
                return false;
            }
        }

        void CandidateState::sendRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& requestVoteReq = request->request_vote_req();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(request->from_node_id());
            // 应答信息
            auto response = message->mutable_request_vote_resp();
            response->set_follower_term(raft.getCurrentTerm());
            response->set_candidate_term(requestVoteReq.candidate_term());
            response->set_success(success);
            // 发送
            messages.push_back(message);
        }

        bool CandidateState::handleRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& requestVoteResp = request->request_vote_resp();
            // 投票失败
            if (!requestVoteResp.success())
            {
                // 更新任期，转换到跟随者
                if (raft.getCurrentTerm() < requestVoteResp.follower_term())
                {
                    raft.setCurrentTerm(requestVoteResp.follower_term());
                    state_->process_event(DiscoversEvent());
                    return true;
                }
                return false;
            }
            // 投票成功，且是本任期投票
            if (requestVoteResp.candidate_term() == raft.getCurrentTerm())
            {
                votes_.insert(request->from_node_id());
            }
            return false;
        }

        void CandidateState::sendRequestVoteReq(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            for (auto&& destNodeId : raft.getBuddyNodeIds())
            {
                // 路由信息
                auto message = std::make_shared<pb::RaftMsg>();
                message->set_from_node_id(options.getNodeId());
                message->set_dest_node_id(destNodeId);
                // 请求信息
                auto request = message->mutable_request_vote_req();
                request->set_candidate_term(raft.getCurrentTerm());
                request->set_last_log_index(storage->getLastIndex());
                request->set_last_log_term(storage->getLastTerm());
                // 发送
                messages.push_back(message);
            }
        }

        LeaderState::LeaderState()
            : state_(nullptr),
            lastLogIndex_(0)
        {}

        LeaderState::~LeaderState()
        {}

        void LeaderState::on_entry(const MajorityVotesEvent&, RaftState&)
        {
            auto nowTime = state_->getNowTime();
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            // 最大索引
            lastLogIndex_ = storage->getLastIndex();
            // 设置自己是leader
            raft.setLeaderId(options.getNodeId());
            // 设置伙伴节点列表
            auto nextIndex = lastLogIndex_ + 1;
            auto waitIndex = nextIndex;
            nodes_.clear();
            for (auto&& nodeId : raft.getBuddyNodeIds())
            {
                BuddyNode buddyNode(nodeId);
                buddyNode.setNextIndex(nextIndex);
                buddyNode.setWaitIndex(waitIndex);
                nodes_.push_back(buddyNode);
            }
            // 设置心跳时间
            heatbeatTime_ = nowTime;
        }

        void LeaderState::set_sm_ptr(RaftState* sm)
        {
            state_ = sm;
        }

        std::uint64_t LeaderState::update(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto nowTime = state_->getNowTime();
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            // 心跳超时
            if (heatbeatTime_ <= nowTime)
            {
                broadcastAppendEntriesReq(messages);
                heatbeatTime_ = nowTime + options.getHeatbeatTimeout();
            }
            // 处理消息
            auto& messageQueue = state_->getMessages();
            while (!messageQueue.empty())
            {
                auto message = messageQueue.front();
                messageQueue.pop_front();
                // 追加日志
                if (message->has_append_entries_req())
                {
                    if (handleAppendEntriesReq(message, messages))
                    {
                        return nowTime;
                    }
                }
                // 追加日志回复
                else if (message->has_append_entries_resp())
                {
                    if (handleAppendEntriesResp(message, messages))
                    {
                        return nowTime;
                    }
                }
                // 投票请求
                else if (message->has_request_vote_req())
                {
                    if (handleRequestVoteReq(message, messages))
                    {
                        return nowTime;
                    }
                }
                else if (message->has_request_vote_resp())
                {

                }
            }
            // 计算commitIndex
            std::vector<std::uint64_t> commitIndexs;
            commitIndexs.push_back(storage->getLastIndex());
            for (auto&& buddy : nodes_)
            {
                commitIndexs.push_back(buddy.getMatchIndex());
            }
            std::sort(commitIndexs.begin(), commitIndexs.end(), std::greater<std::uint64_t>());
            auto commitIndex = commitIndexs[(1 + nodes_.size()) / 2];
            // 更新commitIndex
            if (commitIndex > raft.getCommitIndex() && commitIndex >= lastLogIndex_)
            {
                raft.setCommitIndex(commitIndex);
            }
            // 传输日志
            transferAppendEntriesReq(messages);
            // 等待心跳
            return heatbeatTime_;
        }

        bool LeaderState::handleAppendEntriesReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& appendEntriesReq = request->append_entries_req();
            if (appendEntriesReq.leader_term() > raft.getCurrentTerm())
            {
                auto& messageQueue = state_->getMessages();
                messageQueue.push_front(request);
                state_->process_event(DiscoversEvent());
                return true;
            }
            else
            {
                sendAppendEntriesResp(request, false, messages);
                return false;
            }
        }

        void LeaderState::sendAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& appendEntriesReq = request->append_entries_req();
            auto appendLogIndex = appendEntriesReq.prev_log_index() + appendEntriesReq.entries_size();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(request->from_node_id());
            // 应答信息
            auto response = message->mutable_append_entries_resp();
            response->set_follower_term(raft.getCurrentTerm());
            response->set_leader_term(appendEntriesReq.leader_term());
            response->set_prev_log_index(appendEntriesReq.prev_log_index());
            response->set_append_log_index(appendLogIndex);
            response->set_last_log_index(storage->getLastIndex());
            response->set_success(success);
            // 发送
            messages.push_back(message);
        }

        bool LeaderState::handleRequestVoteReq(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& requestVoteReq = request->request_vote_req();
            if (requestVoteReq.candidate_term() > raft.getCurrentTerm())
            {
                auto& messageQueue = state_->getMessages();
                messageQueue.push_front(request);
                state_->process_event(DiscoversEvent());
                return true;
            }
            else
            {
                sendRequestVoteResp(request, false, messages);
                return false;
            }
        }

        void LeaderState::sendRequestVoteResp(const std::shared_ptr<pb::RaftMsg>& request, bool success, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& requestVoteReq = request->request_vote_req();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(request->from_node_id());
            // 应答信息
            auto response = message->mutable_request_vote_resp();
            response->set_follower_term(raft.getCurrentTerm());
            response->set_candidate_term(requestVoteReq.candidate_term());
            response->set_success(success);
            // 发送
            messages.push_back(message);
        }

        bool LeaderState::handleAppendEntriesResp(const std::shared_ptr<pb::RaftMsg>& request, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto& appendEntriesResp = request->append_entries_resp();
            // 任期过期
            if (appendEntriesResp.follower_term() > raft.getCurrentTerm())
            {
                raft.setCurrentTerm(appendEntriesResp.follower_term());
                state_->process_event(DiscoversEvent());
                return true;
            }
            // 任期不对
            if (appendEntriesResp.leader_term() != raft.getCurrentTerm())
            {
                return false;
            }
            // 来自的节点
            auto nodeIter = std::find_if(nodes_.begin(), nodes_.end(), [&](const BuddyNode& buddy)
            {
                return buddy.getNodeId() == request->from_node_id();
            });
            if (nodeIter == nodes_.end())
            {
                return false;
            }
            auto nextIndex = nodeIter->getNextIndex();
            auto waitIndex = nodeIter->getWaitIndex();
            auto waitEntriesNum = nodeIter->getWaitEntriesNum();
            auto matchIndex = nodeIter->getMatchIndex();
            // 日志过期
            auto prevLogIndex = appendEntriesResp.prev_log_index();
            auto appendLogIndex = appendEntriesResp.append_log_index();
            if (prevLogIndex + 1 < waitIndex || appendLogIndex >= nextIndex)
            {
                return false;
            }
            // 追加日志失败
            if (!appendEntriesResp.success())
            {
                nextIndex = std::min({ nextIndex - 1, appendEntriesResp.last_log_index(), prevLogIndex });
                nextIndex = std::max<std::uint64_t>(nextIndex, 1);
                waitIndex = nextIndex;
                waitEntriesNum = 1;
                matchIndex = std::min(matchIndex, nextIndex - 1);
            }
            else
            {
                nextIndex = std::max({ nextIndex, appendLogIndex + 1, appendEntriesResp.last_log_index() + 1 });
                nextIndex = std::min(nextIndex, storage->getLastIndex() + 1);
                waitIndex = std::max({ waitIndex, appendLogIndex + 1, appendEntriesResp.last_log_index() + 1 });
                waitIndex = std::min(waitIndex, nextIndex);
                waitEntriesNum = std::min(waitEntriesNum * 2, options.getMaxWaitEntriesNum());
                matchIndex = std::max(matchIndex, appendLogIndex);
            }
            // 设置新的索引
            nodeIter->setNextIndex(nextIndex);
            nodeIter->setWaitIndex(waitIndex);
            nodeIter->setWaitEntriesNum(waitEntriesNum);
            nodeIter->setMatchIndex(matchIndex);
            // 追加成功
            return false;
        }

        void LeaderState::sendAppendEntriesReq(BuddyNode& buddy, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto lastLogIndex = storage->getLastIndex();
            auto maxPacketNum = options.getMaxPacketLength();
            // 路由信息
            auto message = std::make_shared<pb::RaftMsg>();
            message->set_from_node_id(options.getNodeId());
            message->set_dest_node_id(buddy.getNodeId());
            // 领导者信息
            auto appendEntriesReq = message->mutable_append_entries_req();
            appendEntriesReq->set_leader_term(raft.getCurrentTerm());
            appendEntriesReq->set_leader_commit(raft.getCommitIndex());
            // 日志信息
            auto nextLogIndex = buddy.getNextIndex();
            auto waitLogIndex = buddy.getWaitIndex();
            auto waitLogNum = buddy.getWaitEntriesNum();
            auto prevLogIndex = nextLogIndex - ((nextLogIndex == waitLogIndex + waitLogNum) ? 2 : 1);
            auto prevLogTerm = storage->getTermByIndex(prevLogIndex);
            appendEntriesReq->set_prev_log_index(prevLogIndex);
            appendEntriesReq->set_prev_log_term(prevLogTerm);
            // 追加的日志
            if (prevLogIndex + 1 <= lastLogIndex && prevLogIndex + 1 <= waitLogIndex + waitLogNum - 1)
            {
                auto stopLogIndex = std::min(lastLogIndex, waitLogIndex + waitLogNum - 1);
                auto entries = storage->getEntries(prevLogIndex + 1, stopLogIndex, maxPacketNum);
                auto appendEntries = appendEntriesReq->mutable_entries();
                for (auto&& entry : entries)
                {
                    *appendEntries->Add() = entry;
                }
                // 更新发送给索引
                nextLogIndex = prevLogIndex + 1 + entries.size();
                buddy.setNextIndex(nextLogIndex);
            }
            // 发送
            messages.push_back(message);
        }

        void LeaderState::transferAppendEntriesReq(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            auto& raft = state_->getRaft();
            auto& options = raft.getOptions();
            auto& storage = options.getStorage();
            auto lastLogIndex = storage->getLastIndex();
            for (auto&& node : nodes_)
            {
                auto nextLogIndex = node.getNextIndex();
                auto waitLogIndex = node.getWaitIndex();
                auto waitLogNum = node.getWaitEntriesNum();
                while (nextLogIndex <= lastLogIndex && nextLogIndex < waitLogIndex + waitLogNum)
                {
                    // 发送日志
                    sendAppendEntriesReq(node, messages);
                    // 更新索引
                    nextLogIndex = node.getNextIndex();
                    waitLogIndex = node.getWaitIndex();
                }
            }
        }

        void LeaderState::broadcastAppendEntriesReq(std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            for (auto&& node : nodes_)
            {
                sendAppendEntriesReq(node, messages);
            }
        }

    }
}