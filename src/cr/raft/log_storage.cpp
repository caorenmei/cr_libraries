#include <cr/raft/log_storage.h>

namespace cr
{
    namespace raft
    {
        LogStorage::LogStorage() 
        {}

        LogStorage::~LogStorage() 
        {}

        bool LogStorage::isSupportSnapshot() const
        {
            return false;
        }

        LogStorage::Result LogStorage::putSnapshot(std::uint32_t instanceId, const Snapshot& snapshot)
        {
            return NO_SUPPORT;
        }

        LogStorage::Result LogStorage::getSnapshot(std::uint32_t instanceId, Snapshot& snapshot)
        {
            return NO_SUPPORT;
        }
    }
}