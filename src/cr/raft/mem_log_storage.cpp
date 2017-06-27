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

        int MemLogStorage::getAllInstanceId(std::vector<std::uint32_t>& instanceIds)
        {
            for (auto&& instance : logs_)
            {
                instanceIds.push_back(instance.first);
            }
            return error::SUCCESS;
        }

        int MemLogStorage::get(std::uint32_t instanceId, std::uint64_t logIndex, LogEntry& logEntry)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end())
            {
                return error::NO_INSTANCE;
            }
            const auto& logEntries = instanceIter->second;
            if (logIndex >= logEntries.size())
            {
                return error::NO_LOG_INDEX;
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(logIndex);
            logEntry = logEntries[vecLogIndex];
            return error::SUCCESS;
        }

        int MemLogStorage::append(std::uint32_t instanceId, const LogEntry& logEntry)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end() && logEntry.getIndex() == 0)
            {
                instanceIter = logs_.insert(decltype(logs_)::value_type(instanceId, {})).first;
            }
            if (instanceIter == logs_.end())
            {
                return error::NO_INSTANCE;
            }
            auto& logEntries = instanceIter->second;
            if (logEntry.getIndex() != logEntries.size())
            {
                return error::LOG_INDEX_ERROR;
            }
            logEntries.emplace_back(logEntry);
            return error::SUCCESS;
        }

        int MemLogStorage::del(std::uint32_t instanceId, std::uint64_t startLogIndex)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end())
            {
                return error::NO_INSTANCE;
            }
            auto& logEntries = instanceIter->second;
            if (startLogIndex >= logEntries.size())
            {
                return error::LOG_INDEX_ERROR;
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(startLogIndex);
            logEntries.erase(logEntries.begin() + vecLogIndex, logEntries.end());
            return error::SUCCESS;
        }

        int MemLogStorage::del(std::uint32_t instanceId)
        {
            logs_.erase(instanceId);
            return error::SUCCESS;
        }

        int MemLogStorage::getLastLogIndex(std::uint32_t instanceId, std::uint64_t& logIndex)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end())
            {
                return error::NO_INSTANCE;
            }
            auto& logEntries = instanceIter->second;
            if (logEntries.empty())
            {
                return error::NO_LOG_DATA;
            }
            logIndex = static_cast<std::uint64_t>(logEntries.size() - 1);
            return error::SUCCESS;
        }
    }
}