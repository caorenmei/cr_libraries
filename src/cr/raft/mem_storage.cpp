#include <cr/raft/mem_storage.h>

#include <algorithm>

#include <cr/common/throw.h>
#include <cr/raft/exception.h>

namespace cr
{
    namespace raft
    {

        void MemStorage::append(const Entry& entry)
        {
            if (entry.getIndex() == entries_.size() && entry.getTerm() >= lastTerm())
            {
                entries_.push_back(entry);
            }
            CR_THROW(cr::raft::StoreException, "Entry Index Out of Bound");
        }

        void MemStorage::remove(std::uint64_t startIndex)
        {
            if(startIndex >= 1 && startIndex <= entries_.size())
            {
                entries_.erase(entries_.begin() + startIndex - 1, entries_.end());
            }
            CR_THROW(cr::raft::StoreException, "Start Index Out of Bound");
        }

        std::vector<Entry> MemStorage::entries(std::uint64_t startIndex, std::uint64_t stopIndex)
        {
            if (startIndex <= stopIndex && startIndex >= 1 && stopIndex <= entries_.size())
            {
                return { entries_.begin() + startIndex - 1, entries_.begin() + stopIndex };
            }
            CR_THROW(cr::raft::StoreException, "Start Index or Stop Index Out of Bound");
        }

        std::uint32_t MemStorage::term(std::uint64_t index)
        {
            if (index <= entries_.size())
            {
                return entries_[static_cast<std::size_t>(index - 1)].getTerm();
            }
            CR_THROW(cr::raft::StoreException, "Log Index Out of Bound");
        }

        std::uint64_t MemStorage::lastIndex()
        {
            return !entries_.empty() ? entries_.size() : 0;
        }

        std::uint32_t MemStorage::lastTerm()
        {
            return !entries_.empty() ? entries_.back().getTerm() : 0;
        }
    }
}