#ifndef CR_RAFT_MEM_LOG_STORAGE_H_
#define CR_RAFT_MEM_LOG_STORAGE_H_

#include <vector>

#include "storage.h"

namespace cr
{
    namespace raft
    {
        /** 内存日志存储接口，主要用于测试 */
        class MemStorage : public Storage
        {
        public:

            /** 构造函数 */
            MemStorage() = default;

            /** 析构函数 */
            virtual ~MemStorage() noexcept override = default;

            MemStorage(const MemStorage&) = delete;
            MemStorage& operator=(const MemStorage&) = delete;

            /**
             * 追加日志
             * @param entries 日志条目
             */
            virtual void append(const std::vector<pb::Entry>& entry) override;

            /**
             * 删除日志
             * @param startIndex 日志起始索引
             */
            virtual void remove(std::uint64_t startIndex) override;

            /**
             * 获取日志
             * @param startIndex 日志起始索引
             * @param stopIndex 日志结束索引
             * @param maxPacketLength 数据最大小
             * @return 日志条目
             */
            virtual std::vector<pb::Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) const override;

            /**
             * 获取日志编号对应的任期
             * @param index 日志编号
             */
            virtual std::uint64_t getTermByIndex(std::uint64_t index) const override;

            /**
             * 获取最后的日志编号
             * @return 日志编号
             */
            virtual std::uint64_t getLastIndex() const override;

            /**
             * 获取最后的任期编号
             * @return 任期编号
             */
            virtual std::uint64_t getLastTerm() const override;

        private:

            std::vector<pb::Entry> entries_;
        };
    }
}

#endif
