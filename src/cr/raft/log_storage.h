#ifndef CR_RAFT_LOG_STRAGE_H_
#define CR_RAFT_LOG_STRAGE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace cr
{
    namespace raft
    {
        /**
         * 二进制日志存储接口
         */
        class LogStrage
        {
        public:

            /** 日志接口操作返回类型 */
            enum Result : int
            {
                /** 操作成功 */
                SUCCESS = 0,
                /** 没有该实例 */
                NO_INSTANCE = 1,
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

            /** 构造函数 */
            inline LogStrage() {}

            /** 析构函数 */
            virtual ~LogStrage() {}

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

        };
    }
}

#endif
