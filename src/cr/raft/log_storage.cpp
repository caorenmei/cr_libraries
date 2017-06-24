#include <cr/raft/log_storage.h>

namespace cr
{
    namespace raft
    {
        LogStrage::LogStrage() 
        {}

        LogStrage::~LogStrage() 
        {}

        bool LogStrage::isSupportSnapshot() const
        {
            return false;
        }

        LogStrage::Result LogStrage::putSnapshot(const Snapshot& snapshot)
        {
            return NO_SUPPORT;
        }

        LogStrage::Result LogStrage::getSnapshot(std::uint32_t instanceId, Snapshot& snapshot)
        {
            return NO_SUPPORT;
        }
    }
}