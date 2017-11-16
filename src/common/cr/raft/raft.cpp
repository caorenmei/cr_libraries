#include "raft.h"

#include <cassert>

#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {

        Raft::Raft(const Options& options)
            : state_(std::ref(*this)),
            options_(options),
            buddyNodeIds_(options_.getBuddyNodeIds()),
            random_(options_.getRandomSeed()),
            currentTerm_(0),
            commitIndex_(0),
            lastApplied_(0)
        {}

        Raft::~Raft()
        {}

        const IRaftState& Raft::getState() const
        {
            return state_;
        }

        const Options& Raft::getOptions() const
        {
            return options_;
        }

        const std::vector<std::uint64_t>& Raft::getBuddyNodeIds() const
        {
            return buddyNodeIds_;
        }

        void Raft::start(std::uint64_t nowTime)
        {
            state_.setNowTime(nowTime);
            state_.start(StartUpEvent());
        }

        const boost::optional<std::uint64_t>& Raft::getVotedFor() const
        {
            return votedFor_;
        }

        const boost::optional<std::uint64_t>& Raft::getLeaderId() const
        {
            return leaderId_;
        }

        std::uint64_t Raft::getCurrentTerm() const
        {
            return currentTerm_;
        }

        std::uint64_t Raft::getCommitIndex() const
        {
            return commitIndex_;
        }

        std::uint64_t Raft::getLastApplied() const
        {
            return lastApplied_;
        }

        std::uint64_t Raft::update(std::uint64_t nowTime, std::vector<std::shared_ptr<pb::RaftMsg>>& messages)
        {
            bool isLeader = state_.isLeader();
            // 运行状态机
            nowTime = std::max(nowTime, state_.getNowTime());
            state_.setNowTime(nowTime);
            auto nextTime = state_.update(messages);
            // 不是领导了
            if (isLeader && !state_.isLeader())
            {
                leaderLost();
            }
            // 运行日志
            if (lastApplied_ < commitIndex_)
            {
                applyLog();
            }
            // 继续运行日志
            if (lastApplied_ < commitIndex_)
            {
                nextTime = nowTime;
            }
            return nextTime;
        }

        void Raft::receive(std::shared_ptr<pb::RaftMsg> message)
        {
            state_.getMessages().push_back(std::move(message));
        }

        std::pair<std::uint64_t, bool> Raft::execute(const std::string& value)
        {
            if (state_.isLeader())
            {
                auto& storage = options_.getStorage();
                auto logIndex = storage->getLastIndex();
                // 创建日志
                pb::Entry entry;
                entry.set_index(logIndex + 1);
                entry.set_term(currentTerm_);
                entry.set_value(value);
                // 提交日志
                storage->append({ entry });
                return std::make_pair(entry.index(), true);
            }
            return std::make_pair(0, false);
        }

        std::pair<std::uint64_t, bool> Raft::execute(const std::string& value, std::function<void(std::uint64_t, int)> cb)
        {
            auto result = execute(value);
            if (result.second)
            {
                callbacks_.insert(std::make_pair(result.first, std::move(cb)));
            }
            return result;
        }

        std::mt19937& Raft::getRandom()
        {
            return random_;
        }

        void Raft::setVotedFor(const boost::optional<std::uint64_t>& votedFor)
        {
            votedFor_ = votedFor;
        }

        void Raft::setLeaderId(const boost::optional<std::uint64_t>& leaderId)
        {
            leaderId_ = leaderId;
        }

        void Raft::setCurrentTerm(std::uint64_t currentTerm)
        {
            currentTerm_ = currentTerm;
        }

        void Raft::setCommitIndex(std::uint64_t commitIndex)
        {
            commitIndex_ = commitIndex;
            // 已提交的回调
            std::vector<std::function<void()>> callbacks;
            auto committer = callbacks_.upper_bound(commitIndex);
            for (auto iter = callbacks_.begin(); iter != committer; ++iter)
            {
                callbacks.push_back(std::bind(std::move(iter->second), iter->first, RESULT_COMMITTED));
            }
            callbacks_.erase(callbacks_.begin(), committer);
            // 处理回调
            for (auto& callback : callbacks)
            {
                callback();
            }
        }

        void Raft::applyLog()
        {
            auto& storage = options_.getStorage();
            auto& executable = options_.getEexcutable();
            auto lastLogIndex = std::min(lastApplied_ + 10, commitIndex_);
            auto entries = storage->getEntries(lastApplied_ + 1, lastLogIndex, 0xffffffff);
            for (auto& entry : entries)
            {
                executable(entry.index(), entry.value());
            }
            lastApplied_ = lastApplied_ + entries.size();
        }

        void Raft::leaderLost()
        {
            // 所有回调
            std::vector<std::function<void()>> callbacks;
            for (auto iter = callbacks_.begin(); iter != callbacks_.end(); ++iter)
            {
                callbacks.push_back(std::bind(std::move(iter->second), iter->first, RESULT_COMMITTED));
            }
            callbacks_.clear();
            // 处理回调
            for (auto& callback : callbacks)
            {
                callback();
            }
        }
    }
}