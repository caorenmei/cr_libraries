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
             * 获取所有的实例ID
             * @param [out] instanceIds 实例ID列表
             * @return 操作结果
             */
            virtual int getAllInstanceId(std::vector<std::uint32_t>& instanceIds) = 0;

            /**
             * 获取一条日志
             * @param instanceId 一个raft实例ID
             * @param logIndex 日志ID，从0开始
             * @param [out] logEntry 日志条目
             * @return 操作结果
             */
            virtual int get(std::uint32_t instanceId, std::uint64_t logIndex, LogEntry& logEntry) = 0;

            /**
             * 追加日志
             * @param logEntry 日志
             * @return 操作结果
             */
            virtual int append(std::uint32_t instanceId, const LogEntry& logEntry) = 0;

            /**
             * 删除日志
             * @param instanceId 实例ID
             * @param startLogIndex 起始日志索引
             * @return 操作结果
             */
            virtual int del(std::uint32_t instanceId, std::uint64_t startLogIndex) = 0;

            /**
             * 删除实例的所有日志
             * @param instanceId 实例ID
             * @return 操作结果
             */
            virtual int del(std::uint32_t instanceId) = 0;

            /**
             * 获取最后的日志索引
             * @param instanceId 实例ID
             * @param lastLogIndex 最后的日志索引
             * @return 操作结果
             */
            virtual int getLastLogIndex(std::uint32_t instanceId, std::uint64_t& lastLogIndex) = 0;

        };
    }
}

#endif
