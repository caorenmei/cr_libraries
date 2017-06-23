﻿#ifndef CR_RAFT_LOG_STRAGE_H_
#define CR_RAFT_LOG_STRAGE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace cr
{
    namespace raft
    {
        /** 二进制日志存储接口 */
        class LogStrage
        {
        public:

            /** 日志接口操作返回类型 */
            enum Result : int
            {
                /** 操作成功 */
                SUCCESS,
                /** 不支持的操作 */
                NO_SUPPORT,
                /** 数据损坏 */
                BAD_LOG,
                /** 没有该实例 */
                NO_INSTANCE,
                /** 没有该日志  */
                NO_LOG_INDEX,
                /** 没有快照 */
                NO_SNAPSHOT,
                /** 日志已生成快照 */
                IN_SNPSHOT,
            };

            /** 日志条目 */
            struct LogEntry
            {
                /** 实例ID */
                std::uint32_t instanceId;
                /** 日志ID */
                std::uint64_t logIndex;
                /** 任期ID*/
                std::uint32_t termId;
                /** 日志条目 */
                std::string value;
            };

            /** 日志条目指针 */
            using LogEntryPtr = std::shared_ptr<LogEntry>;

            /** 二进制条目 */
            using ByteEntry = std::shared_ptr<std::string>;

            /** 快照接口 */
            struct Snapshot
            {
                /** 实例ID */
                std::uint32_t instanceId;
                /** 日志ID */
                std::uint64_t lastLogIndex;
                /** 任期ID*/
                std::uint32_t lastTermIndex;
                /** 游标 */
                std::function<void(Result, ByteEntry)> cursor;
            };

            /** 快照接口指针 */
            using SnapshotPtr = std::shared_ptr<Snapshot>;

            /** 构造函数 */
            LogStrage();

            /** 析构函数 */
            virtual ~LogStrage();

            LogStrage(const LogStrage&) = delete;
            LogStrage& operator=(const LogStrage&) = delete;

            /**
             * 获取一条日志
             * @param options 读取参数
             * @param instanceId 一个raft实例ID
             * @param logIndex 日志ID，从0开始
             * @param handler 异步返回接口
             */
            virtual void get(std::uint32_t instanceId, std::uint64_t logIndex, std::function<void(Result, LogEntryPtr)> handler) = 0;

            /**
             * 追加日志
             * @param options 写入参数
             * @param entry 日志
             * @param handler 异步返回接口
             */
            virtual void append(LogEntryPtr entry, std::function<void(Result)> handler) = 0;

            /**
             * 删除日志
             * @param instanceId 实例ID
             * @param startLogIndex 起始日志索引
             * @param handler 异步返回接口
             */
            virtual void del(std::uint32_t instanceId, std::uint64_t startLogIndex, std::function<void(Result)> handler) = 0;

            /**
             * 删除实例的所有日志
             * @param instanceId 实例ID
             * @param handler 异步返回接口
             */
            virtual void del(std::uint32_t instanceId, std::function<void(Result)> handler) = 0;

            /**
             * 获取最后的日志索引
             * @param instanceId 实例ID
             * @param handler 异步返回接口
             */
            virtual void getLastLogIndex(std::uint32_t instanceId, std::function<void(Result, std::uint64_t)> handler) = 0;

            /** 
             * 是否支持快照 
             * @return True支持快照，False其它
             */
            virtual bool isSupportSnapshot() const;

            /** 
             * 写入快照快照
             * @param snapshot 快照接口
             * @param handler 异步返回接口
             */
            virtual void putSnapshot(SnapshotPtr snapshot, std::function<void(Result)> handler);

            /**
             * 读取快照
             * @param instanceId 实例ID
             * @param handler 异步返回接口
             */
            virtual void getSnapshot(std::uint32_t instanceId, std::function<void(Result, SnapshotPtr)> handler);

        };
    }
}

#endif
