#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/error_code.h>
#include <cr/raft/mem_log_storage.h>

BOOST_AUTO_TEST_SUITE(MemLogStorage)

BOOST_AUTO_TEST_CASE(get)
{
    cr::raft::MemLogStorage storage;

    cr::raft::LogEntry getLogEntry;

    BOOST_CHECK_EQUAL(storage.get(0, getLogEntry), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.get(1, getLogEntry), cr::raft::error::LOG_INDEX_ERROR);

    storage.append(cr::raft::LogEntry(1, 1, "1"));
    BOOST_CHECK_EQUAL(storage.get(0, getLogEntry), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.get(1, getLogEntry), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(getLogEntry.getIndex(), 1);
    BOOST_CHECK_EQUAL(getLogEntry.getTerm(), 1);
    BOOST_CHECK_EQUAL(getLogEntry.getValue(), "1");

    storage.del(1);
    BOOST_CHECK_EQUAL(storage.get(0, getLogEntry), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.get(1, getLogEntry), cr::raft::error::LOG_INDEX_ERROR);
}

BOOST_AUTO_TEST_CASE(append)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_EQUAL(storage.append(cr::raft::LogEntry(0, 1, "")), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.append(cr::raft::LogEntry(2, 1, "")), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.append(cr::raft::LogEntry(1, 1, "")), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(storage.append(cr::raft::LogEntry(2, 1, "")), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(storage.append(cr::raft::LogEntry(4, 1, "")), cr::raft::error::LOG_INDEX_ERROR);
}

BOOST_AUTO_TEST_CASE(del)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_EQUAL(storage.del(0), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.del(1), cr::raft::error::LOG_INDEX_ERROR);

    storage.append(cr::raft::LogEntry(1, 1, ""));

    BOOST_CHECK_EQUAL(storage.del(2), cr::raft::error::LOG_INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.del(1), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(storage.del(0), cr::raft::error::LOG_INDEX_ERROR);
}

BOOST_AUTO_TEST_CASE(getLastIndex)
{
    cr::raft::MemLogStorage storage;

    std::uint64_t lastLogIndex;
    BOOST_CHECK_EQUAL(storage.getLastIndex(lastLogIndex), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(lastLogIndex, 0);

    storage.append(cr::raft::LogEntry(1, 1, ""));
    BOOST_CHECK_EQUAL(storage.getLastIndex(lastLogIndex), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(lastLogIndex, 1);

    storage.append(cr::raft::LogEntry(2, 1, ""));
    BOOST_CHECK_EQUAL(storage.getLastIndex(lastLogIndex), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(lastLogIndex, 2);

    storage.del(1);
    BOOST_CHECK_EQUAL(storage.getLastIndex(lastLogIndex), cr::raft::error::SUCCESS);
    BOOST_CHECK_EQUAL(lastLogIndex, 0);
}

BOOST_AUTO_TEST_SUITE_END()
