#ifndef CR_RAFT_MEM_LOG_STORAGE_H_
#define CR_RAFT_MEM_LOG_STORAGE_H_

#include <unordered_map>
#include <vector>

#include <cr/raft/log_storage.h>

namespace cr
{
    namespace raft
    {
        /** 内存日志存储接口，主要用于测试 */
        class MemLogStorage : public LogStorage
        {
        public:

            /** 构造函数 */
            MemLogStorage();

            /** 析构函数 */
            ~MemLogStorage();

            MemLogStorage(const MemLogStorage&) = delete;
            MemLogStorage& operator=(const MemLogStorage&) = delete;

            /**
             * 获取一条日志
             * @param logIndex 日志ID，从1开始
             * @param [out] logEntry 日志条目
             * @return 操作结果
             */
            virtual int get(std::uint64_t logIndex, LogEntry& logEntry) override;

            /**
             * 追加日志
             * @param logEntry 日志
             * @return 操作结果
             */
            virtual int append(const LogEntry& logEntry) override;

            /**
             * 删除日志
             * @param startLogIndex 起始日志索引
             * @return 操作结果
             */
            virtual int del(std::uint64_t startIndex) override;

            /**
             * 获取最后的日志索引
             * @param lastLogIndex 最后的日志索引
             * @return 操作结果
             */
            virtual int getLastIndex(std::uint64_t& lastIndex) override;

        private:

            std::vector<LogEntry> logEntries_;
        };
    }
}

#endif
