#include "file_storage.h"

#include <memory>

#include <boost/endian/arithmetic.hpp>
#include <boost/lexical_cast.hpp>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>

#include <cr/core/assert.h>
#include <cr/core/throw.h>

#include "exception.h"
#include "raft_msg.pb.h"

namespace
{
    //  整数到字符串
    std::string uInt64ToString(std::uint64_t value)
    {
        boost::endian::native_to_little_inplace(value);
        return std::string(reinterpret_cast<char*>(&value), sizeof(value));
    }

    // 字符串到整数
    std::uint64_t stringToUInt64(const std::string& str)
    {
        assert(str.size() == sizeof(std::uint64_t));
        std::uint64_t value;
        std::memcpy(&value, str.data(), sizeof(value));
        return boost::endian::little_to_native(value);
    }

    // 解析数据项
    cr::raft::pb::Entry sliceToEntry(const rocksdb::Slice& slice)
    {
        cr::raft::pb::Entry entry;
        entry.ParseFromArray(slice.data(), slice.size());
        return entry;
    }
}

namespace cr
{
    namespace raft
    {
        
        FileStorage::FileStorage(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* columnFamily, cr::log::Logger& logger, bool sync/* = true*/)
            : db_(db),
            columnFamily_(columnFamily),
            logger_(logger),
            sync_(sync),
            lastLogIndex_(0),
            lastTermIndex_(0)
        {
            auto iter = std::unique_ptr<rocksdb::Iterator>(db_->NewIterator(rocksdb::ReadOptions(), columnFamily_));
            iter->SeekToLast();
            if (iter->Valid())
            {
                auto entry = sliceToEntry(iter->value());
                lastLogIndex_ = entry.index();
                lastTermIndex_ = entry.term();
            }
        }

        FileStorage::FileStorage(rocksdb::DB* db, cr::log::Logger& logger, bool sync/* = true*/)
            : FileStorage(db, db->DefaultColumnFamily(), logger, sync)
        {}

        FileStorage::~FileStorage() noexcept
        {}

        void FileStorage::append(const std::vector<pb::Entry>& entries)
        {
            CRLOG_DEBUG(logger_, "FileStorage") << "append: entries.size()=" << entries.size();
            auto lastLogIndex = lastLogIndex_;
            auto lastTermIndex = lastTermIndex_;
            rocksdb::WriteBatch writeBatch;
            for (auto& entry : entries)
            {
                lastLogIndex = lastLogIndex + 1;
                assert(lastLogIndex == entry.index());
                lastTermIndex = entry.term();
                writeBatch.Put(columnFamily_, uInt64ToString(lastLogIndex), entry.SerializeAsString());
            }
            rocksdb::WriteOptions wtiteOptions;
            wtiteOptions.sync = sync_;
            auto status = db_->Write(wtiteOptions, &writeBatch);
            if (!status.ok())
            {
                CRLOG_ERROR(logger_, "FileStorage") << "append faield: " << lastLogIndex_;
                CR_THROW(IOException, "File storage append fialed");
            }
            lastLogIndex_ = lastLogIndex;
            lastTermIndex_ = lastTermIndex;
        }

        void FileStorage::remove(std::uint64_t startIndex)
        {
            CRLOG_DEBUG(logger_, "FileStorage") << "remove: start " << startIndex << " to " << lastLogIndex_;
            assert(startIndex >= 0 && startIndex <= lastLogIndex_);
            rocksdb::WriteBatch writeBatch;
            for (auto logIndex = startIndex; logIndex <= lastLogIndex_; ++logIndex)
            {
                writeBatch.Delete(columnFamily_, uInt64ToString(logIndex));
            }
            rocksdb::WriteOptions wtiteOptions;
            wtiteOptions.sync = sync_;
            auto status = db_->Write(wtiteOptions, &writeBatch);
            if (!status.ok())
            {
                CRLOG_ERROR(logger_, "FileStorage") << "Delete Entry Start faield: " << startIndex;
                CR_THROW(IOException, "File storage remove fialed");
            }
            if (startIndex != 1)
            {
                lastTermIndex_ = getTermByIndex(startIndex - 1);
                lastLogIndex_ = startIndex - 1;
            }
            else
            {
                lastLogIndex_ = 0;
                lastTermIndex_ = 0;
            }
        }

        std::vector<pb::Entry> FileStorage::getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) const
        {
            assert(startIndex > 0 && startIndex <= lastLogIndex_ && startIndex <= stopIndex);
            stopIndex = std::min(stopIndex, lastLogIndex_);
            maxPacketLength = std::max<std::uint64_t>(maxPacketLength, 1);
            std::uint64_t packetLength = 0;
            std::vector<pb::Entry> entries;
            for (auto logIndex = startIndex; logIndex <= stopIndex && packetLength < maxPacketLength; ++logIndex)
            {
                std::string buffer;
                auto status = db_->Get(rocksdb::ReadOptions(), columnFamily_, uInt64ToString(logIndex), &buffer);
                if (!status.ok() || status.IsNotFound())
                {
                    CRLOG_ERROR(logger_, "FileStorage") << "Get term by index failed : " << logIndex;
                    CR_THROW(IOException, "Get term by index failed");
                }
                auto entry = sliceToEntry(buffer);
                packetLength = packetLength + entry.value().size();
                entries.push_back(std::move(entry));
            }
            return entries;
        }

        std::uint64_t FileStorage::getTermByIndex(std::uint64_t index) const
        {
            assert(index <= lastLogIndex_);
            if (index == lastLogIndex_)
            {
                return lastTermIndex_;
            }
            std::string buffer;
            auto status = db_->Get(rocksdb::ReadOptions(), columnFamily_, uInt64ToString(index), &buffer);
            if (!status.ok() || status.IsNotFound())
            {
                CRLOG_ERROR(logger_, "FileStorage") << "Get term by index failed : " << index;
                CR_THROW(IOException, "Get term by index failed");
            }
            auto entry = sliceToEntry(buffer);
            return entry.term();
        }

        std::uint64_t FileStorage::getLastIndex() const
        {
            return lastLogIndex_;
        }

        std::uint64_t FileStorage::getLastTerm() const
        {
            return lastTermIndex_;
        }
    }
}