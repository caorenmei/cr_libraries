#ifndef CR_RAFT_LOG_STRAGE_H_
#define CR_RAFT_LOG_STRAGE_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace cr
{
    namespace raft
    {
        /** 日志条目 */
        class LogEntry
        {
        public:

            /** 构造函数 */
            LogEntry();

            /**
             * 构造函数
             * @param index 日志索引
             * @param term 任期编号
             * @param value 日志内容
             */
            LogEntry(std::uint64_t index, std::uint32_t term, std::string value);

            /** 析构函数 */
            ~LogEntry();

            /**
             * 获取日志索引编号
             * @return 日志索引编号
             */
            std::uint64_t getIndex() const;

            /**
             * 设置日志索引编号
             * @param 日志索引编号
             */
            void setIndex(std::uint64_t index);

            /**
             * 获取日志任期编号
             * @return 日志任期编号 
             */
            std::uint32_t getTerm() const;

            /**
             * 设置日志任期编号
             * @param 日志任期编号
             */
            void setTerm(std::uint32_t term);

            /**
             * 获取日志值
             * @return 日志值 
             */
            const std::string& getValue() const;

            /**
             * 设置日志值
             * @param 日志值
             */
            void setValue(std::string value);

        private:

            // 日志ID
            std::uint64_t index_;
            // 任期ID/
            std::uint32_t term_;
            // 日志条目
            std::string value_;
        };

        /** 二进制日志存储接口 */
        class LogStorage
        {
        public:

            /** 构造函数 */
            LogStorage();

            /** 析构函数 */
            virtual ~LogStorage();

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
