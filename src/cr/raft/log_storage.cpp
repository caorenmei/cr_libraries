#include <cr/raft/log_storage.h>

namespace cr
{
    namespace raft
    {
        /** 构造函数 */
        LogStrage::LogStrage() 
        {}

        /** 析构函数 */
        LogStrage::~LogStrage() 
        {}

        /**
        * 是否支持快照
        * @return True支持快照，False其它
        */
        bool LogStrage::isSupportSnapshot() const
        {
            return false;
        }

        /**
        * 写入快照快照
        * @param snapshot 快照接口
        * @param handler 异步返回接口
        */
        void LogStrage::putSnapshot(SnapshotPtr snapshot, std::function<void(Result)> handler)
        {
            handler(NO_SUPPORT);
        }

        /**
        * 读取快照
        * @param instanceId 实例ID
        * @param handler 异步返回接口
        */
        void LogStrage::getSnapshot(std::uint32_t instanceId, std::function<void(Result, SnapshotPtr)> handler)
        {
            handler(NO_SUPPORT, nullptr);
        }
    }
}