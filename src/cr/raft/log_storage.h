#ifndef CR_RAFT_LOG_STRAGE_H_
#define CR_RAFT_LOG_STRAGE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <cr/raft/log_entry.h>

namespace cr
{
    namespace raft
    {
        /** 二进制日志存储接口 */
        class LogStorage
        {
        public:

            /** 构造函数 */
            inline LogStorage() {}

            /** 析构函数 */
            virtual ~LogStorage() {}

            LogStorage(const LogStorage&) = delete;
            LogStorage& operator=(const LogStorage&) = delete;

            /**
             * 获取一条日志
             * @param logIndex 日志ID，从1开始
             * @param [out] logEntry 日志条目
             * @exception StoreException 异常发生
             */
            virtual void get(std::uint64_t logIndex, LogEntry& logEntry) = 0;

            /**
             * 追加日志
             * @param logEntry 日志
             * @exception StoreException 异常发生
             */
            virtual void append(const LogEntry& logEntry) = 0;

            /**
             * 删除日志
             * @param startLogIndex 起始日志索引
             * @exception StoreException 异常发生
             */
            virtual void del(std::uint64_t startIndex) = 0;

            /**
             * 获取最后的日志索引
             * @return lastLogIndex 最后的日志索引
             * @exception StoreException 异常发生
             */
            virtual std::uint64_t getLastIndex() = 0;

        };
    }
}

#endif
