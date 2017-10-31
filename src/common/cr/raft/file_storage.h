#ifndef CR_COMMON_FILE_STORAGE_H_
#define CR_COMMON_FILE_STORAGE_H_

#include <cstdint>
#include <memory>

#include <rocksdb/db.h>

#include <cr/core/logging.h>

#include "storage.h"

namespace cr
{
    namespace raft
    {
        /** 保存日志到文件存储 */
        class FileStorage : public Storage
        {
        public:

            /**
             * 构造函数
             * @param 
             */
            explicit FileStorage(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* columnFamily, cr::log::Logger& logger, bool sync = true);

            /** 析构函数 */
            virtual ~FileStorage() noexcept override;

            FileStorage(const FileStorage&) = delete;
            FileStorage& operator=(const FileStorage&) = delete;

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

            // rocksdb
            rocksdb::DB& db_;
            rocksdb::ColumnFamilyHandle* columnFamily_;
            // 日志
            cr::log::Logger& logger_;
            // 同步标志
            bool sync_;
            // 最大索引
            std::uint64_t lastLogIndex_;
            // 最大任期
            std::uint64_t lastTermIndex_;
        };
    }
}

#endif
