#ifndef CR_RAFT_LOG_ENTRY_H_
#define CR_RAFT_LOG_ENTRY_H_

#include <cstdint>
#include <string>

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
    }
}

#endif
