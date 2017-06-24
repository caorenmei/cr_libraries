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

            /** 日志接口操作返回类型 */
            enum Result : int
            {
                /** 操作成功 */
                SUCCESS,
                /** 不支持的操作 */
                NO_SUPPORT,
                /** 序号错误 */
                INDEX_ERROR,
                /** 日志数据错误 */
                LOG_ERROR,
                /** 没有该实例 */
                NO_INSTANCE,
                /** 没有该日志  */
                NO_LOG_INDEX,
                /** 实例没有日志  */
                NO_LOG,
                /** 没有快照 */
                NO_SNAPSHOT,
                /** 日志已生成快照 */
                IN_SNPSHOT,
            };

            /** 二进制条目 */
            using ByteEntry = std::shared_ptr<std::string>;

            /** 快照接口 */
            struct Snapshot
            {
                /** 日志ID */
                std::uint64_t lastIndex;
                /** 任期ID*/
                std::uint32_t lastTerm;
                /** 游标 */
                std::function<void(Result, ByteEntry)> cursor;
            };

            /** 快照接口指针 */
            using SnapshotPtr = std::shared_ptr<Snapshot>;

            /** 构造函数 */
            LogStorage();

            /** 析构函数 */
            virtual ~LogStorage();

            LogStorage(const LogStorage&) = delete;
            LogStorage& operator=(const LogStorage&) = delete;

            /** 
             * 获取所有的实例ID
             * @param [out] instanceIds 实例ID列表
             * @return 操作结果
             */
            virtual Result getAllInstanceId(std::vector<std::uint32_t>& instanceIds) = 0;

            /**
             * 获取一条日志
             * @param instanceId 一个raft实例ID
             * @param logIndex 日志ID，从0开始
             * @param [out] logEntry 日志条目
             * @return 操作结果
             */
            virtual Result get(std::uint32_t instanceId, std::uint64_t logIndex, LogEntry& logEntry) = 0;

            /**
             * 追加日志
             * @param logEntry 日志
             * @return 操作结果
             */
            virtual Result append(std::uint32_t instanceId, const LogEntry& logEntry) = 0;

            /**
             * 删除日志
             * @param instanceId 实例ID
             * @param startLogIndex 起始日志索引
             * @return 操作结果
             */
            virtual Result del(std::uint32_t instanceId, std::uint64_t startLogIndex) = 0;

            /**
             * 删除实例的所有日志
             * @param instanceId 实例ID
             * @return 操作结果
             */
            virtual Result del(std::uint32_t instanceId) = 0;

            /**
             * 获取最后的日志索引
             * @param instanceId 实例ID
             * @param lastLogIndex 最后的日志索引
             * @return 操作结果
             */
            virtual Result getLastLogIndex(std::uint32_t instanceId, std::uint64_t& lastLogIndex) = 0;

            /** 
             * 是否支持快照 
             * @return True支持快照，False其它
             */
            virtual bool isSupportSnapshot() const;

            /** 
             * 写入快照快照
             * @param snapshot 快照接口
             * @return 操作结果
             */
            virtual Result putSnapshot(std::uint32_t instanceId, const Snapshot& snapshot);

            /**
             * 读取快照
             * @param instanceId 实例ID
             * @param [out] snapshot 快照接口
             * @return 操作结果
             */
            virtual Result getSnapshot(std::uint32_t instanceId, Snapshot& snapshot);

        };
    }
}

#endif
