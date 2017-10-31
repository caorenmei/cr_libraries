#include "mem_storage.h"

#include <cassert>
#include <algorithm>

#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {
        void MemStorage::append(const std::vector<pb::Entry>& entries)
        {
            auto lastLogIdex = !entries_.empty() ? entries_.back().index() : 0;
            for (auto&& entry : entries)
            {
                lastLogIdex = lastLogIdex + 1;
                assert(entry.index() == lastLogIdex);
                entries_.push_back(entry);
            }
        }

        void MemStorage::remove(std::uint64_t startIndex)
        {
            assert(startIndex <= entries_.size());
            startIndex = std::max<std::uint64_t>(startIndex, 1);
            auto logIndex = static_cast<std::size_t>(startIndex - 1);
            entries_.erase(entries_.begin() + logIndex, entries_.end());
        }

        std::vector<pb::Entry> MemStorage::getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) const
        {
            assert(startIndex > 0 && startIndex <= stopIndex && startIndex <= entries_.size());
            std::vector<pb::Entry> results;
            maxPacketLength = std::max<std::uint64_t>(maxPacketLength, 1);
            std::uint64_t packetLength = 0;
            auto logIndex = static_cast<std::size_t>(startIndex - 1);
            auto stopLogIndex = std::min<std::size_t>(stopIndex, entries_.size());
            for (; logIndex < stopLogIndex && packetLength < maxPacketLength; ++logIndex)
            {
                const auto& entry = entries_[logIndex];
                results.push_back(entry);
                packetLength += entry.ByteSize();
            }
            return results;
        }

        std::uint64_t MemStorage::getTermByIndex(std::uint64_t index) const
        {
            assert(index <= entries_.size());
            index = std::max<std::uint64_t>(index, 1);
            auto logIndex = static_cast<std::size_t>(index - 1);
            return entries_[logIndex].term();
        }

        std::uint64_t MemStorage::getLastIndex() const
        {
            return !entries_.empty() ? entries_.back().index() : 0;
        }

        std::uint64_t MemStorage::getLastTerm() const
        {
            return !entries_.empty() ? entries_.back().term() : 0;
        }
    }
}