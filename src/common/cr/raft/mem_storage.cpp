#include "mem_storage.h"

#include <algorithm>

#include <cr/core/assert.h>

#include "exception.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {

        void MemStorage::append(std::uint64_t startIndex, const std::vector<pb::Entry>& entries)
        {
            std::uint64_t lastIndex = getLastIndex();
            std::uint64_t lastTerm = getLastTerm();
            CR_ASSERT_E(cr::raft::ArgumentException, startIndex == lastIndex + 1)(startIndex)(lastIndex);
            for (auto&& entry : entries)
            {
                CR_ASSERT_E(cr::raft::ArgumentException, entry.term() >= lastTerm)(entry.term())(lastTerm);
                entries_.push_back(entry);
                lastIndex = lastIndex + 1;
                lastTerm = entry.term();
            }
        }

        void MemStorage::remove(std::uint64_t startIndex)
        {
            CR_ASSERT_E(cr::raft::ArgumentException, startIndex >= 1 && startIndex <= getLastIndex())(startIndex)(getLastIndex());
            entries_.erase(entries_.begin() + startIndex - 1, entries_.end());
        }

        std::vector<pb::Entry> MemStorage::getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength)
        {
            auto lastLogIndex = getLastIndex();
            CR_ASSERT_E(cr::raft::ArgumentException, startIndex >=1 && startIndex <= stopIndex && stopIndex <= lastLogIndex)(startIndex)(stopIndex)(lastLogIndex);
            std::vector<pb::Entry> results;
            results.reserve(static_cast<std::size_t>(stopIndex - startIndex + 1));
            maxPacketLength = std::max<std::uint64_t>(maxPacketLength, 1);
            std::uint64_t packetLength = 0;
            for (auto logIndex = startIndex; logIndex <= stopIndex && packetLength < maxPacketLength; ++logIndex)
            {
                const auto& entry = entries_[static_cast<std::size_t>(logIndex - 1)];
                results.emplace_back(entry);
                packetLength += entry.ByteSize();
            }
            return results;
        }

        std::uint64_t MemStorage::getTermByIndex(std::uint64_t index)
        {
            CR_ASSERT_E(cr::raft::ArgumentException, index >= 1 && index <= getLastIndex())(index)(getLastIndex());
            return entries_[static_cast<std::size_t>(index - 1)].term();
        }

        std::uint64_t MemStorage::getLastIndex()
        {
            return !entries_.empty() ? entries_.size() : 0;
        }

        std::uint64_t MemStorage::getLastTerm()
        {
            return !entries_.empty() ? entries_.back().term() : 0;
        }
    }
}