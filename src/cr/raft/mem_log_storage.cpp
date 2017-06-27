#include <cr/raft/mem_log_storage.h>

#include <cr/raft/error_code.h>

namespace cr
{
    namespace raft
    {
        MemLogStorage::MemLogStorage()
        {}

        MemLogStorage::~MemLogStorage()
        {}

        int MemLogStorage::get(std::uint64_t logIndex, LogEntry& logEntry)
        {
            if (logIndex < 1 || logIndex > logEntries_.size())
            {
                return error::LOG_INDEX_ERROR;
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(logIndex);
            logEntry = logEntries_[vecLogIndex - 1];
            return error::SUCCESS;
        }

        int MemLogStorage::append(const LogEntry& logEntry)
        {
            if (logEntry.getIndex() < 1 || logEntry.getIndex() > logEntries_.size() + 1)
            {
                return error::LOG_INDEX_ERROR;
            }
            logEntries_.emplace_back(logEntry);
            return error::SUCCESS;
        }

        int MemLogStorage::del(std::uint64_t startIndex)
        {
            if (startIndex < 1 || startIndex > logEntries_.size())
            {
                return error::LOG_INDEX_ERROR;
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(startIndex);
            logEntries_.erase(logEntries_.begin() + vecLogIndex - 1, logEntries_.end());
            return error::SUCCESS;
        }

        int MemLogStorage::getLastIndex(std::uint64_t& logIndex)
        {
            logIndex = static_cast<std::uint64_t>(logEntries_.size());
            return error::SUCCESS;
        }
    }
}