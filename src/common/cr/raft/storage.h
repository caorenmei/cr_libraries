#ifndef CR_RAFT_STORAGE_H_
#define CR_RAFT_STORAGE_H_

#include <cstdint>
#include <string>
#include <vector>

namespace cr
{
    namespace raft
    {
        namespace pb
        {
            /** 日志条目 */
            class Entry;
        }

        /** 存储接口 */
        class Storage
        {
        public:

            /** 构造函数 */
            Storage() = default;

            /** 析构函数 */
            virtual ~Storage() noexcept = default;

            /** 
             * 追加日志
             * @param entries 日志条目
             */
            virtual void append(const std::vector<pb::Entry>& entries) = 0;

            /**
             * 删除日志
             * @param startIndex 日志起始索引
             */
            virtual void remove(std::uint64_t startIndex) = 0;

            /**
             * 获取日志
             * @param startIndex 日志起始索引
             * @param stopIndex 日志结束索引
             * @param maxPacketLength 数据最大小
             * @return 日志条目
             */
            virtual std::vector<pb::Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) const = 0;

            /**
             * 获取日志编号对应的任期
             * @param index 日志编号
             */
            virtual std::uint64_t getTermByIndex(std::uint64_t index) const = 0;

            /**
             * 获取最后的日志编号
             * @return 日志编号
             */
            virtual std::uint64_t getLastIndex() const = 0;

            /**
             * 获取最后的任期编号
             * @return 任期编号
             */
            virtual std::uint64_t getLastTerm() const = 0;
        };
    }
}

#endif
