#include <cr/raft/mem_storage.h>

#include <algorithm>

#include <cr/common/assert.h>
#include <cr/raft/exception.h>

namespace cr
{
    namespace raft
    {

        void MemStorage::append(const std::vector<Entry>& entries)
        {
            for (auto&& entry : entries)
            {
                CR_ASSERT_E(cr::raft::ArgumentException, entry.getIndex() == getLastIndex() + 1 && entry.getTerm() >= getLastTerm())(entry.getIndex())(entry.getTerm())(getLastIndex())(getLastTerm());
                entries_.push_back(entry);
            }
        }

        void MemStorage::remove(std::uint64_t startIndex)
        {
            CR_ASSERT_E(cr::raft::ArgumentException, startIndex >= 1 && startIndex <= getLastIndex())(startIndex)(getLastIndex());
            entries_.erase(entries_.begin() + startIndex - 1, entries_.end());
        }

        std::vector<Entry> MemStorage::getEntries(std::uint64_t startIndex, std::uint64_t stopIndex)
        {
            CR_ASSERT_E(cr::raft::ArgumentException, startIndex >=1 && startIndex <= stopIndex && stopIndex <= getLastIndex())(startIndex)(stopIndex)(getLastIndex());
            return { entries_.begin() + startIndex - 1, entries_.begin() + stopIndex };
        }

        std::uint64_t MemStorage::getTermByIndex(std::uint64_t index)
        {
            CR_ASSERT_E(cr::raft::ArgumentException, index >= 1 && index <= getLastIndex())(index)(getLastIndex());
            return entries_[static_cast<std::size_t>(index - 1)].getTerm();
        }

        std::uint64_t MemStorage::getLastIndex()
        {
            return !entries_.empty() ? entries_.size() : 0;
        }

        std::uint64_t MemStorage::getLastTerm()
        {
            return !entries_.empty() ? entries_.back().getTerm() : 0;
        }
    }
}