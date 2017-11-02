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

        RaftState& Raft::getState()
        {
            return state_;
        }

        const RaftState& Raft::getState() const
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

        bool Raft::propose(const std::string& value)
        {
            return propose(std::vector<std::string>{ value });
        }

        bool Raft::propose(const std::vector<std::string>& values)
        {
            if (state_.isLeader())
            {
                auto& storage = options_.getStorage();
                auto logIndex = storage->getLastIndex();
                std::vector<pb::Entry> entries;
                for (auto&& value : values)
                {
                    logIndex = logIndex + 1;
                    entries.emplace_back();
                    auto& entry = entries.back();
                    entry.set_index(logIndex);
                    entry.set_term(currentTerm_);
                    entry.set_value(value);
                }
                storage->append(entries);
                return true;
            }
            return false;
        }

        bool Raft::execute(std::size_t logEntryNum/* = 10*/)
        {
            if (lastApplied_ < commitIndex_)
            {
                auto& storage = options_.getStorage();
                auto& executable = options_.getEexcutable();
                auto entries = storage->getEntries(lastApplied_ + 1, lastApplied_ + logEntryNum, 0xffffffff);
                for (auto& entry : entries)
                {
                    assert(entry.index() == lastApplied_ + 1);
                    executable(entry.index(), entry.value());
                    lastApplied_ = lastApplied_ + 1;
                }
                return lastApplied_ < commitIndex_;
            }
            return false;
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
        }
    }
}