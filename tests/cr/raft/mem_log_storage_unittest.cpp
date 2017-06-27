#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/error_code.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_log_storage.h>

BOOST_AUTO_TEST_SUITE(MemLogStorage)

BOOST_AUTO_TEST_CASE(get)
{
    cr::raft::MemLogStorage storage;

    cr::raft::LogEntry getLogEntry;

    BOOST_CHECK_THROW(storage.get(0, getLogEntry), cr::raft::StoreException);
    BOOST_CHECK_THROW(storage.get(1, getLogEntry), cr::raft::StoreException);

    storage.append(cr::raft::LogEntry(1, 1, "1"));
    BOOST_CHECK_THROW(storage.get(0, getLogEntry), cr::raft::StoreException);
    BOOST_CHECK_NO_THROW(storage.get(1, getLogEntry));
    BOOST_CHECK_EQUAL(getLogEntry.getIndex(), 1);
    BOOST_CHECK_EQUAL(getLogEntry.getTerm(), 1);
    BOOST_CHECK_EQUAL(getLogEntry.getValue(), "1");

    storage.del(1);
    BOOST_CHECK_THROW(storage.get(0, getLogEntry), cr::raft::StoreException);
    BOOST_CHECK_THROW(storage.get(1, getLogEntry), cr::raft::StoreException);
}

BOOST_AUTO_TEST_CASE(append)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_THROW(storage.append(cr::raft::LogEntry(0, 1, "")), cr::raft::StoreException);
    BOOST_CHECK_THROW(storage.append(cr::raft::LogEntry(2, 1, "")), cr::raft::StoreException);
    BOOST_CHECK_NO_THROW(storage.append(cr::raft::LogEntry(1, 1, "")));
    BOOST_CHECK_NO_THROW(storage.append(cr::raft::LogEntry(2, 1, "")));
    BOOST_CHECK_THROW(storage.append(cr::raft::LogEntry(4, 1, "")), cr::raft::StoreException);
}

BOOST_AUTO_TEST_CASE(del)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_NO_THROW(storage.del(0));
    BOOST_CHECK_THROW(storage.del(1), cr::raft::StoreException);

    storage.append(cr::raft::LogEntry(1, 1, ""));

    BOOST_CHECK_THROW(storage.del(2), cr::raft::StoreException);
    BOOST_CHECK_NO_THROW(storage.del(1));
    BOOST_CHECK_NO_THROW(storage.del(0));
}

BOOST_AUTO_TEST_CASE(getLastIndex)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_EQUAL(storage.getLastIndex(), 0);

    storage.append(cr::raft::LogEntry(1, 1, ""));
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 1);

    storage.append(cr::raft::LogEntry(2, 1, ""));
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 2);

    storage.del(1);
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
