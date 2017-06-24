#include <cr/raft/mem_log_storage.h>

namespace cr
{
    namespace raft
    {
        MemLogStorage::MemLogStorage()
        {}

        MemLogStorage::~MemLogStorage()
        {}

        MemLogStorage::Result MemLogStorage::getAllInstanceId(std::vector<std::uint32_t>& instanceIds)
        {
            for (auto&& instance : logs_)
            {
                instanceIds.push_back(instance.first);
            }
            return SUCCESS;
        }

        MemLogStorage::Result MemLogStorage::get(std::uint32_t instanceId, std::uint64_t logIndex, LogEntry& logEntry)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end())
            {
                return NO_INSTANCE;
            }
            const auto& logEntries = instanceIter->second;
            if (logIndex >= logEntries.size())
            {
                return NO_LOG_INDEX;
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(logIndex);
            logEntry = logEntries[vecLogIndex];
            return SUCCESS;
        }

        MemLogStorage::Result MemLogStorage::append(std::uint32_t instanceId, const LogEntry& logEntry)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end() && logEntry.getIndex() == 0)
            {
                instanceIter = logs_.insert(decltype(logs_)::value_type(instanceId, {})).first;
            }
            if (instanceIter == logs_.end())
            {
                return NO_INSTANCE;
            }
            auto& logEntries = instanceIter->second;
            if (logEntry.getIndex() != logEntries.size())
            {
                return INDEX_ERROR;
            }
            logEntries.emplace_back(logEntry);
            return SUCCESS;
        }

        MemLogStorage::Result MemLogStorage::del(std::uint32_t instanceId, std::uint64_t startLogIndex)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end())
            {
                return NO_INSTANCE;
            }
            auto& logEntries = instanceIter->second;
            if (startLogIndex >= logEntries.size())
            {
                return INDEX_ERROR;
            }
            std::size_t vecLogIndex = static_cast<std::size_t>(startLogIndex);
            logEntries.erase(logEntries.begin() + vecLogIndex, logEntries.end());
            return SUCCESS;
        }

        MemLogStorage::Result MemLogStorage::del(std::uint32_t instanceId)
        {
            logs_.erase(instanceId);
            return SUCCESS;
        }

        MemLogStorage::Result MemLogStorage::getLastLogIndex(std::uint32_t instanceId, std::uint64_t& logIndex)
        {
            auto instanceIter = logs_.find(instanceId);
            if (instanceIter == logs_.end())
            {
                return NO_INSTANCE;
            }
            auto& logEntries = instanceIter->second;
            if (logEntries.empty())
            {
                return NO_LOG;
            }
            logIndex = static_cast<std::uint64_t>(logEntries.size() - 1);
            return SUCCESS;
        }
    }
}