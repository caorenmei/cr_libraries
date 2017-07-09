#include <cr/raft/mem_storage.h>

#include <algorithm>

#include <cr/common/assert.h>
#include <cr/raft/exception.h>

namespace cr
{
    namespace raft
    {

        void MemStorage::append(const Entry& entry)
        {
            CR_ASSERT_E(cr::raft::StoreException, entry.getIndex() == lastIndex() + 1 && entry.getTerm() >= lastTerm())(entry.getIndex())(entry.getTerm())(lastIndex())(lastTerm());
            entries_.push_back(entry);
        }

        void MemStorage::remove(std::uint64_t startIndex)
        {
            CR_ASSERT_E(cr::raft::StoreException, startIndex >= 1 && startIndex <= lastIndex())(startIndex)(lastIndex());
            entries_.erase(entries_.begin() + startIndex - 1, entries_.end());
        }

        std::vector<Entry> MemStorage::entries(std::uint64_t startIndex, std::uint64_t stopIndex)
        {
            CR_ASSERT_E(cr::raft::StoreException, startIndex >=1 && startIndex <= stopIndex && stopIndex <= lastIndex())(startIndex)(stopIndex)(lastIndex());
            return { entries_.begin() + startIndex - 1, entries_.begin() + stopIndex };
        }

        std::uint64_t MemStorage::term(std::uint64_t index)
        {
            CR_ASSERT_E(cr::raft::StoreException, index >= 1 && index <= lastIndex())(index)(lastIndex());
            return entries_[static_cast<std::size_t>(index - 1)].getTerm();
        }

        std::uint64_t MemStorage::lastIndex()
        {
            return !entries_.empty() ? entries_.size() : 0;
        }

        std::uint64_t MemStorage::lastTerm()
        {
            return !entries_.empty() ? entries_.back().getTerm() : 0;
        }
    }
}