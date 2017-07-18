#include <cr/raft/file_storage.h>

#include <array>
#include <map>
#include <mutex>

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/lexical_cast.hpp>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>

#include <cr/common/assert.h>
#include <cr/common/throw.h>
#include <cr/raft/exception.h>
#include <cr/raft/raft_msg.pb.h>

namespace cr
{
    namespace raft
    {

        static std::string uint64ToString(std::uint64_t value)
        {
            boost::endian::native_to_little_inplace(value);
            return std::string(reinterpret_cast<char*>(&value), sizeof(value));
        }

        static std::uint64_t stringToUint64(const std::string& value)
        {
            std::uint64_t result = 0;
            CR_ASSERT_E(cr::raft::IOException, value.size() == sizeof(result));
            std::memcpy(&result, value.data(), sizeof(result));
            boost::endian::little_to_native_inplace(result);
            return result;
        }

        static std::string getLogValueKey(std::uint64_t index)
        {
            return "value_" + uint64ToString(index);
        }

        static std::string getLogTermKey(std::uint64_t index)
        {
            return "term_" + uint64ToString(index);
        }

        static std::string getLastLogIndexTermKey()
        {
            return "last_log_index_term";
        }

        static std::string getLastLogIndexTermValue(std::uint64_t key, std::uint64_t value)
        {
            boost::endian::native_to_little_inplace(key);
            boost::endian::native_to_little_inplace(value);
            std::array<std::uint64_t, 2> buffer = { key, value };
            return std::string(reinterpret_cast<char*>(buffer.data()), sizeof(buffer));
        }

        static std::pair<std::uint64_t, std::uint64_t> parseLastLogIndexTermValue(const std::string& value)
        {
            std::array<std::uint64_t, 2> result = {};
            CR_ASSERT_E(cr::raft::IOException, value.size() == sizeof(result))(value.size())(sizeof(result));
            std::memcpy(result.data(), value.data(), value.size());
            for (auto& value : result)
            {
                boost::endian::little_to_native_inplace(value);
            }
            return { result[0], result[1] };
        }


        static std::string getColumnFamilyNamePrefix()
        {
            return "instance_";
        }

        static std::string getColumnFamilyName(std::uint64_t instance)
        {
            return getColumnFamilyNamePrefix() + std::to_string(instance);
        }

        class RocksdbStorage : public Storage
        {
        public:
            RocksdbStorage(std::shared_ptr<rocksdb::DB> db, std::shared_ptr<rocksdb::ColumnFamilyHandle> column, bool sync)
                : db_(db),
                column_(column),
                sync_(sync)
            {
                std::string lastLogIndexTermValue;
                auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLastLogIndexTermKey()), &lastLogIndexTermValue);
                CR_ASSERT_E(cr::raft::IOException, status.ok() || status.IsNotFound())(status.ok())(status.IsNotFound());
                std::tie(lastLogIndex_, lastLogTerm_) = !lastLogIndexTermValue.empty() ? parseLastLogIndexTermValue(lastLogIndexTermValue ) : std::make_pair(0, 0);
            }

            ~RocksdbStorage()
            {}

            virtual void append(std::uint64_t startIndex, const std::vector<pb::Entry>& entries) override
            {
                auto db = db_.lock();
                auto column = column_.lock();
                CR_ASSERT_E(cr::raft::IOException, db != nullptr && column != nullptr)(column.get())(column.get());

                CR_ASSERT_E(cr::raft::ArgumentException, startIndex == lastLogIndex_ + 1)(startIndex)(lastLogIndex_);
                auto lastLogIndex = lastLogIndex_;
                auto lastLogTerm = lastLogTerm_;
                rocksdb::WriteBatch batch;
                std::string buffer;
                for (auto&& entry : entries)
                {
                    CR_ASSERT_E(cr::raft::ArgumentException, entry.term() >= lastLogTerm)(entry.term())(lastLogTerm);
                    lastLogIndex = lastLogIndex + 1;
                    entry.SerializeToString(&buffer);
                    batch.Put(column.get(), rocksdb::Slice(getLogValueKey(lastLogIndex)), rocksdb::Slice(buffer));
                    buffer.clear();
                    lastLogTerm = entry.term();
                    batch.Put(column.get(), rocksdb::Slice(getLogTermKey(lastLogIndex)), rocksdb::Slice(uint64ToString(lastLogTerm)));
                }
                batch.Put(column.get(), rocksdb::Slice(getLastLogIndexTermKey()), rocksdb::Slice(getLastLogIndexTermValue(lastLogIndex, lastLogTerm)));

                rocksdb::WriteOptions writeOptions;
                writeOptions.sync = sync_;
                auto status = db->Write(writeOptions, &batch);
                CR_ASSERT_E(cr::raft::IOException, status.ok());

                lastLogIndex_ = lastLogIndex;
                lastLogTerm_ = lastLogTerm;
            }

            virtual void remove(std::uint64_t startIndex) override
            {
                auto db = db_.lock();
                auto column = column_.lock(); 
                CR_ASSERT_E(cr::raft::IOException, db != nullptr && column != nullptr)(column.get())(column.get());
                CR_ASSERT_E(cr::raft::ArgumentException, startIndex >= 1 && startIndex <= lastLogIndex_)(startIndex)(lastLogTerm_);

                auto lastLogIndex = startIndex - 1;
                auto lastLogTerm = lastLogIndex != 0 ? lastLogTerm_ : 0;
                if (lastLogTerm != 0)
                {
                    std::string lastLogTermValue;
                    auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogTermKey(lastLogIndex)), &lastLogTermValue);
                    CR_ASSERT_E(cr::raft::IOException, status.ok())(status.ok());
                    lastLogTerm = stringToUint64(lastLogTermValue);
                }

                rocksdb::WriteBatch batch;
                for (auto logIndex = startIndex; logIndex <= lastLogIndex; ++logIndex)
                {
                    batch.Delete(column.get(), rocksdb::Slice(getLogValueKey(logIndex)));
                    batch.Delete(column.get(), rocksdb::Slice(getLogTermKey(logIndex)));
                }
                batch.Put(column.get(), rocksdb::Slice(getLastLogIndexTermKey()), rocksdb::Slice(getLastLogIndexTermValue(lastLogIndex, lastLogTerm)));

                rocksdb::WriteOptions writeOptions;
                writeOptions.sync = sync_;
                auto status = db->Write(writeOptions, &batch);
                CR_ASSERT_E(cr::raft::IOException, status.ok());

                lastLogIndex_ = lastLogIndex;
                lastLogTerm_ = lastLogTerm;
            }

            virtual std::vector<pb::Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) override
            {
                auto db = db_.lock();
                auto column = column_.lock();
                CR_ASSERT_E(cr::raft::IOException, db != nullptr && column != nullptr)(column.get())(column.get());
                CR_ASSERT_E(cr::raft::ArgumentException, startIndex >= 1 && startIndex <= stopIndex && stopIndex <= lastLogIndex_)(startIndex)(stopIndex)(lastLogIndex_);

                std::vector<pb::Entry> results;
                results.reserve(static_cast<std::size_t>(stopIndex - startIndex + 1));
                maxPacketLength = std::max<std::uint64_t>(maxPacketLength, 1);
                std::uint64_t packetLength = 0;
                std::string buffer;
                for (auto logIndex = startIndex; logIndex <= stopIndex && packetLength < maxPacketLength; ++logIndex)
                {
                    results.emplace_back();
                    auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogValueKey(logIndex)), &buffer);
                    CR_ASSERT_E(cr::raft::IOException, status.ok() && results.back().ParseFromString(buffer))(status.ok())(buffer.size());
                    packetLength = packetLength + buffer.size();
                    buffer.clear();
                }

                return results;
            }

            virtual std::uint64_t getTermByIndex(std::uint64_t index) override
            {
                auto db = db_.lock();
                auto column = column_.lock();
                CR_ASSERT_E(cr::raft::IOException, db != nullptr && column != nullptr)(column.get())(column.get());
                CR_ASSERT_E(cr::raft::ArgumentException, index >= 1 && index <= lastLogIndex_)(index)(lastLogIndex_);

                if (index != lastLogIndex_)
                {
                    std::string logTermValue;
                    auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogTermKey(index)), &logTermValue);
                    CR_ASSERT_E(cr::raft::IOException, status.ok())(status.ok());
                    return stringToUint64(logTermValue);
                }
                return lastLogTerm_;
            }

            virtual std::uint64_t getLastIndex() override
            {
                return lastLogIndex_;
            }

            virtual std::uint64_t getLastTerm() override
            {
                return lastLogTerm_;
            }

            std::weak_ptr<rocksdb::DB> db_;
            std::weak_ptr<rocksdb::ColumnFamilyHandle> column_;
            bool sync_;
            std::uint64_t lastLogIndex_;
            std::uint64_t lastLogTerm_;
        };

        class FileStorage::Impl
        {
        public:
            rocksdb::ColumnFamilyOptions options;
            bool sync;
            std::shared_ptr<rocksdb::DB> db;
            std::map<std::string, std::shared_ptr<rocksdb::ColumnFamilyHandle>> columnFamilies;
            std::map<std::uint64_t, std::shared_ptr<Storage>> instances;
            std::mutex mutex;
        };

        FileStorage::FileStorage(const std::string& path, bool sync/* = true*/)
            : impl_(std::make_unique<Impl>())
        {
            impl_->sync = sync;

            rocksdb::DBOptions options;
            options.create_if_missing = true;

            std::vector<std::string> columnFamilyNames;
            auto status = rocksdb::DB::ListColumnFamilies(options, path, &columnFamilyNames);
            CR_ASSERT_E(cr::raft::IOException, status.ok() || status.IsIOError());
            std::vector<rocksdb::ColumnFamilyDescriptor> existsColumnFamilies;
            for (const auto& columnFamilyName : columnFamilyNames)
            {
                existsColumnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(columnFamilyName, impl_->options));
            }
            if (existsColumnFamilies.empty())
            {
                existsColumnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(rocksdb::kDefaultColumnFamilyName, impl_->options));
            }

            rocksdb::DB* db = nullptr;
            std::vector<rocksdb::ColumnFamilyHandle*> columnFamilyHandlers;
            status = rocksdb::DB::Open(options, path, existsColumnFamilies, &columnFamilyHandlers, &db);
            CR_ASSERT_E(cr::raft::IOException, status.ok());
            impl_->db.reset(db);
            for (auto columnFamilyHandler : columnFamilyHandlers)
            {
                impl_->columnFamilies[columnFamilyHandler->GetName()].reset(columnFamilyHandler);
            } 
        }

        FileStorage::~FileStorage()
        {}

        std::shared_ptr<Storage> FileStorage::getStorage(std::uint64_t instanceId)
        {
            std::lock_guard<std::mutex> locker(impl_->mutex);
            auto instanceIter = impl_->instances.find(instanceId);
            if (instanceIter == impl_->instances.end())
            {
                auto columnFamilyName = getColumnFamilyName(instanceId);
                auto columnFamilyIter = impl_->columnFamilies.find(columnFamilyName);
                if (columnFamilyIter == impl_->columnFamilies.end())
                {
                    rocksdb::ColumnFamilyHandle* column = nullptr;
                    auto status = impl_->db->CreateColumnFamily(impl_->options, columnFamilyName, &column);
                    CR_ASSERT_E(cr::raft::IOException, status.ok());
                    std::shared_ptr<rocksdb::ColumnFamilyHandle> columnFamily(column);
                    columnFamilyIter = impl_->columnFamilies.insert(std::make_pair(columnFamilyName, columnFamily)).first;
                }
                auto storage = std::make_shared<RocksdbStorage>(impl_->db, columnFamilyIter->second, impl_->sync);
                instanceIter = impl_->instances.insert(std::make_pair(instanceId, storage)).first;
            }
            return instanceIter->second;
        }
    }
}