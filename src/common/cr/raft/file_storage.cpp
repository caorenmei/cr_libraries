#include "file_storage.h"

#include <array>
#include <map>
#include <mutex>

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/lexical_cast.hpp>
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>

#include <cr/core/assert.h>
#include <cr/core/throw.h>

#include "exception.h"
#include "raft_msg.pb.h"

namespace cr
{
    namespace raft
    {
        FileStorage::FileStorage(rocksdb::DB* db, rocksdb::ColumnFamilyHandle* columnFamily, cr::log::Logger& logger, bool sync = true)
        {}

        FileStorage::~FileStorage() noexcept
        {}

        void FileStorage::append(const std::vector<pb::Entry>& entry)
        {

        }

        void FileStorage::remove(std::uint64_t startIndex)
        {

        }

        std::vector<pb::Entry> FileStorage::getEntries(std::uint64_t startIndex, std::uint64_t stopIndex, std::uint64_t maxPacketLength) const
        {

        }

        std::uint64_t FileStorage::getTermByIndex(std::uint64_t index) const
        {

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