#include <cr/raft/file_storage.h>

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

namespace cr
{
    namespace raft
    {

        static std::string getLogValueKey(std::uint64_t index)
        {
            return "value_" + std::to_string(index);
        }

        static std::string getLogTermKey(std::uint64_t index)
        {
            return "term_" + std::to_string(index);
        }

        static std::string getLastLogIndexKey()
        {
            return "last_log_index";
        }

        static std::string getLastLogTermKey()
        {
            return "last_log_term";
        }

        static std::string uint64ToString(std::uint64_t value)
        {
            return std::to_string(value);
        }

        static std::uint64_t stringToUint64(const std::string& value)
        {
            try
            {
                return boost::lexical_cast<std::uint64_t>(value);
            }
            catch (boost::bad_lexical_cast& e)
            {
                CR_THROW(cr::raft::StateException, e.what());
            }
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
            RocksdbStorage(std::shared_ptr<rocksdb::DB> db, std::shared_ptr<rocksdb::ColumnFamilyHandle> column)
                : db_(db),
                column_(column)
            {
                std::string lastLogIndexValue;
                auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLastLogIndexKey()), &lastLogIndexValue);
                CR_ASSERT_E(cr::raft::StoreException, status.ok() || status.IsNotFound())(status.ok())(status.IsNotFound());
                lastLogIndex_ = !lastLogIndexValue.empty() ? stringToUint64(lastLogIndexValue) : 0;

                std::string lastLogTermValue;
                status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLastLogTermKey()), &lastLogTermValue);
                CR_ASSERT_E(cr::raft::StoreException, status.ok() || status.IsNotFound())(status.ok())(status.IsNotFound());
                lastLogTerm_ = !lastLogTermValue.empty() ? stringToUint64(lastLogTermValue) : 0;
            }

            ~RocksdbStorage()
            {}

            virtual void append(const std::vector<Entry>& entries) override
            {
                auto db = db_.lock();
                auto column = column_.lock();
                CR_ASSERT_E(cr::raft::StoreException, db != nullptr && column != nullptr)(column.get())(column.get());

                auto lastLogIndex = lastLogIndex_;
                auto lastLogTerm = lastLogTerm_;
                rocksdb::WriteBatch batch;
                for (auto&& entry : entries)
                {
                    CR_ASSERT_E(cr::raft::ArgumentException, entry.getIndex() == lastLogIndex + 1 && entry.getTerm() >= lastLogTerm)(entry.getIndex())(entry.getTerm())(lastLogIndex)(lastLogTerm);
                    lastLogIndex = entry.getIndex();
                    batch.Put(column.get(), rocksdb::Slice(getLogValueKey(lastLogIndex)), rocksdb::Slice(entry.getValue()));
                    lastLogTerm = entry.getTerm();
                    batch.Put(column.get(), rocksdb::Slice(getLogTermKey(lastLogIndex)), rocksdb::Slice(uint64ToString(lastLogTerm)));
                }
                batch.Put(column.get(), rocksdb::Slice(getLastLogIndexKey()), rocksdb::Slice(uint64ToString(lastLogIndex)));
                batch.Put(column.get(), rocksdb::Slice(getLastLogTermKey()), rocksdb::Slice(uint64ToString(lastLogTerm)));

                rocksdb::WriteOptions writeOptions;
                writeOptions.sync = true;
                auto status = db->Write(writeOptions, &batch);
                CR_ASSERT_E(cr::raft::StoreException, status.ok());

                lastLogIndex_ = lastLogIndex;
                lastLogTerm_ = lastLogTerm;
            }

            virtual void remove(std::uint64_t startIndex) override
            {
                auto db = db_.lock();
                auto column = column_.lock(); 
                CR_ASSERT_E(cr::raft::StoreException, db != nullptr && column != nullptr)(column.get())(column.get());
                CR_ASSERT_E(cr::raft::ArgumentException, startIndex >= 1 && startIndex <= lastLogIndex_)(startIndex)(lastLogTerm_);

                auto lastLogIndex = startIndex - 1;
                auto lastLogTerm = lastLogIndex != 0 ? lastLogTerm_ : 0;
                if (lastLogTerm != 0)
                {
                    std::string lastLogTermValue;
                    auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogTermKey(lastLogIndex)), &lastLogTermValue);
                    CR_ASSERT_E(cr::raft::StoreException, status.ok())(status.ok());
                    lastLogTerm = stringToUint64(lastLogTermValue);
                }

                rocksdb::WriteBatch batch;
                for (auto logIndex = startIndex; logIndex <= lastLogIndex; ++logIndex)
                {
                    batch.Delete(column.get(), rocksdb::Slice(getLogValueKey(logIndex)));
                    batch.Delete(column.get(), rocksdb::Slice(getLogTermKey(logIndex)));
                }
                batch.Put(column.get(), rocksdb::Slice(getLastLogIndexKey()), rocksdb::Slice(uint64ToString(lastLogIndex)));
                batch.Put(column.get(), rocksdb::Slice(getLastLogTermKey()), rocksdb::Slice(uint64ToString(lastLogTerm)));

                rocksdb::WriteOptions writeOptions;
                writeOptions.sync = true;
                auto status = db->Write(writeOptions, &batch);
                CR_ASSERT_E(cr::raft::StoreException, status.ok());

                lastLogIndex_ = lastLogIndex;
                lastLogTerm_ = lastLogTerm;
            }

            virtual std::vector<Entry> getEntries(std::uint64_t startIndex, std::uint64_t stopIndex) override
            {
                auto db = db_.lock();
                auto column = column_.lock();
                CR_ASSERT_E(cr::raft::StoreException, db != nullptr && column != nullptr)(column.get())(column.get());
                CR_ASSERT_E(cr::raft::ArgumentException, startIndex >= 1 && startIndex <= stopIndex && stopIndex <= lastLogIndex_)(startIndex)(stopIndex)(lastLogIndex_);

                std::vector<Entry> results;
                for (auto logIndex = startIndex; logIndex <= stopIndex; ++logIndex)
                {
                    auto logTerm = lastLogTerm_;
                    std::string logTermValue;
                    auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogTermKey(logIndex)), &logTermValue);
                    CR_ASSERT_E(cr::raft::StoreException, status.ok())(status.ok());
                    logTerm = stringToUint64(logTermValue);

                    std::string logValue;
                    status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogValueKey(logIndex)), &logValue);
                    CR_ASSERT_E(cr::raft::StoreException, status.ok());
                    
                    results.emplace_back(logIndex, logTerm, std::move(logValue));
                }

                return results;
            }

            virtual std::uint64_t getTermByIndex(std::uint64_t index) override
            {
                auto db = db_.lock();
                auto column = column_.lock();
                CR_ASSERT_E(cr::raft::StoreException, db != nullptr && column != nullptr)(column.get())(column.get());
                CR_ASSERT_E(cr::raft::ArgumentException, index >= 1 && index <= lastLogIndex_)(index)(lastLogIndex_);

                if (index != lastLogIndex_)
                {
                    std::string logTermValue;
                    auto status = db->Get(rocksdb::ReadOptions(), column.get(), rocksdb::Slice(getLogTermKey(index)), &logTermValue);
                    CR_ASSERT_E(cr::raft::StoreException, status.ok())(status.ok());
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
            std::uint64_t lastLogIndex_;
            std::uint64_t lastLogTerm_;
        };

        class FileStorage::Impl
        {
        public:
            std::shared_ptr<rocksdb::DB> db;
            std::map<std::string, std::shared_ptr<rocksdb::ColumnFamilyHandle>> columnFamilies;
            std::map<std::uint64_t, std::shared_ptr<Storage>> instances;
            std::mutex mutex;
        };

        FileStorage::FileStorage(const std::string& path)
            : impl_(std::make_unique<Impl>())
        {
            rocksdb::DBOptions options;
            options.create_if_missing = true;

            std::vector<std::string> columnFamilyNames;
            auto status = rocksdb::DB::ListColumnFamilies(options, path, &columnFamilyNames);
            CR_ASSERT_E(cr::raft::StateException, status.ok() || status.IsIOError());
            std::vector<rocksdb::ColumnFamilyDescriptor> existsColumnFamilies;
            for (const auto& columnFamilyName : columnFamilyNames)
            {
                existsColumnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(columnFamilyName, rocksdb::ColumnFamilyOptions()));
            }
            if (existsColumnFamilies.empty())
            {
                existsColumnFamilies.push_back(rocksdb::ColumnFamilyDescriptor(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions()));
            }

            rocksdb::DB* db = nullptr;
            std::vector<rocksdb::ColumnFamilyHandle*> columnFamilyHandlers;
            status = rocksdb::DB::Open(options, path, existsColumnFamilies, &columnFamilyHandlers, &db);
            CR_ASSERT_E(cr::raft::StateException, status.ok());
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
                    auto status = impl_->db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), columnFamilyName, &column);
                    CR_ASSERT_E(cr::raft::StateException, status.ok());
                    columnFamilyIter = impl_->columnFamilies.emplace(columnFamilyName, column).first;
                }
                auto storage = std::make_shared<RocksdbStorage>(impl_->db, columnFamilyIter->second);
                instanceIter = impl_->instances.insert(std::make_pair(instanceId, storage)).first;
            }
            return instanceIter->second;
        }
    }
}