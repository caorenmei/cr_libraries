#include <cr/raft/mem_log_storage.h>

#include <algorithm>

#include <cr/common/throw.h>
#include <cr/raft/exception.h>

namespace cr
{
    namespace raft
    {
        MemLogStorage::MemLogStorage()
        {}

        MemLogStorage::~MemLogStorage()
        {}

        void MemLogStorage::get(std::uint64_t logIndex, LogEntry& logEntry)
        {
            if (logIndex < 1 || logIndex > logEntries_.size())
            {
                CR_THROW(StoreException, "Log Index Out Of Bound", error::LOG_INDEX_ERROR);
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(logIndex);
            logEntry = logEntries_[vecLogIndex - 1];
        }

        std::uint32_t MemLogStorage::getTermByIndex(std::uint64_t logIndex)
        {
            if (logIndex < 1 || logIndex > logEntries_.size())
            {
                CR_THROW(StoreException, "Log Index Out Of Bound", error::LOG_INDEX_ERROR);
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(logIndex);
            return logEntries_[vecLogIndex - 1].getTermByIndex();
        }

        void MemLogStorage::append(const LogEntry& logEntry)
        {
            if (logEntry.getIndex() < 1 || logEntry.getIndex() > logEntries_.size() + 1)
            {
                CR_THROW(StoreException, "Log Entry Index Out Of Bound", error::LOG_INDEX_ERROR);
            }
            logEntries_.emplace_back(logEntry);
        }

        void MemLogStorage::del(std::uint64_t startIndex)
        {
            if (startIndex > logEntries_.size())
            {
                CR_THROW(StoreException, "Start Log Index Out Of Bound", error::LOG_INDEX_ERROR);
            }
            startIndex = std::max<std::uint64_t>(startIndex, 1);
            std::size_t vecLogIndex = static_cast<std::size_t>(startIndex);
            logEntries_.erase(logEntries_.begin() + vecLogIndex - 1, logEntries_.end());
        }

        std::uint64_t MemLogStorage::getLastIndex()
        {
            return static_cast<std::uint64_t>(logEntries_.size());
        }
    }
}