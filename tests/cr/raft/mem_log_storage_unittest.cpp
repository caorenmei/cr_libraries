#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/mem_log_storage.h>

BOOST_AUTO_TEST_SUITE(MemLogStorage)

BOOST_AUTO_TEST_CASE(getAllInstanceId)
{
    cr::raft::MemLogStorage storage;
    std::vector<std::uint32_t> instanceIds;

    BOOST_CHECK_EQUAL(storage.getAllInstanceId(instanceIds), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK(instanceIds.empty());

    storage.append(100, cr::raft::LogEntry(0, 1, "1"));

    BOOST_CHECK_EQUAL(storage.getAllInstanceId(instanceIds), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK(cr::from(instanceIds).sorted().equals(cr::from({ 100 })));

    storage.append(200, cr::raft::LogEntry(0, 1, "1"));

    instanceIds.clear();
    BOOST_CHECK_EQUAL(storage.getAllInstanceId(instanceIds), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK(cr::from(instanceIds).sorted().equals(cr::from({ 100, 200 })));

    storage.del(100);

    instanceIds.clear();
    BOOST_CHECK_EQUAL(storage.getAllInstanceId(instanceIds), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK(cr::from(instanceIds).sorted().equals(cr::from({  200 })));

    storage.del(200);

    instanceIds.clear();
    BOOST_CHECK_EQUAL(storage.getAllInstanceId(instanceIds), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK(instanceIds.empty());
}

BOOST_AUTO_TEST_CASE(get)
{
    cr::raft::MemLogStorage storage;

    cr::raft::LogEntry getLogEntry;

    BOOST_CHECK_EQUAL(storage.get(100, 0, getLogEntry), cr::raft::LogStorage::NO_INSTANCE);

    storage.append(100, cr::raft::LogEntry(0, 1, "1"));

    BOOST_CHECK_EQUAL(storage.get(100, 0, getLogEntry), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK_EQUAL(getLogEntry.getIndex(), 0);
    BOOST_CHECK_EQUAL(getLogEntry.getTerm(), 1);
    BOOST_CHECK_EQUAL(getLogEntry.getValue(), "1");

    BOOST_CHECK_EQUAL(storage.get(100, 1, getLogEntry), cr::raft::LogStorage::NO_LOG_INDEX);

    storage.del(100, 0);

    BOOST_CHECK_EQUAL(storage.get(100, 0, getLogEntry), cr::raft::LogStorage::NO_LOG_INDEX);

    storage.del(100);
    BOOST_CHECK_EQUAL(storage.get(100, 0, getLogEntry), cr::raft::LogStorage::NO_INSTANCE);
}

BOOST_AUTO_TEST_CASE(append)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_EQUAL(storage.append(100, cr::raft::LogEntry(1, 1, "")), cr::raft::LogStorage::NO_INSTANCE);

    BOOST_CHECK_EQUAL(storage.append(100, cr::raft::LogEntry(0, 1, "")), cr::raft::LogStorage::SUCCESS);

    BOOST_CHECK_EQUAL(storage.append(100, cr::raft::LogEntry(1, 1, "")), cr::raft::LogStorage::SUCCESS);

    BOOST_CHECK_EQUAL(storage.append(100, cr::raft::LogEntry(3, 1, "")), cr::raft::LogStorage::INDEX_ERROR);
}

BOOST_AUTO_TEST_CASE(del)
{
    cr::raft::MemLogStorage storage;

    BOOST_CHECK_EQUAL(storage.del(100, 0), cr::raft::LogStorage::NO_INSTANCE);

    storage.append(100, cr::raft::LogEntry(0, 1, ""));

    BOOST_CHECK_EQUAL(storage.del(100, 1), cr::raft::LogStorage::INDEX_ERROR);
    BOOST_CHECK_EQUAL(storage.del(100, 0), cr::raft::LogStorage::SUCCESS);

    BOOST_CHECK_EQUAL(storage.del(100), cr::raft::LogStorage::SUCCESS);
}

BOOST_AUTO_TEST_CASE(getLastLogIndex)
{
    constexpr std::uint32_t instanceId = 100;
    cr::raft::MemLogStorage storage;

    std::uint64_t lastLogIndex;
    BOOST_CHECK_EQUAL(storage.getLastLogIndex(instanceId, lastLogIndex), cr::raft::LogStorage::NO_INSTANCE);

    storage.append(instanceId, cr::raft::LogEntry(0, 1, ""));

    BOOST_CHECK_EQUAL(storage.getLastLogIndex(instanceId, lastLogIndex), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK_EQUAL(lastLogIndex, 0);

    storage.append(instanceId, cr::raft::LogEntry(1, 1, ""));

    BOOST_CHECK_EQUAL(storage.getLastLogIndex(instanceId, lastLogIndex), cr::raft::LogStorage::SUCCESS);
    BOOST_CHECK_EQUAL(lastLogIndex, 1);

    storage.del(100, 0);

    BOOST_CHECK_EQUAL(storage.getLastLogIndex(instanceId, lastLogIndex), cr::raft::LogStorage::NO_LOG);

    storage.del(100);
    BOOST_CHECK_EQUAL(storage.getLastLogIndex(instanceId, lastLogIndex), cr::raft::LogStorage::NO_INSTANCE);
}

BOOST_AUTO_TEST_SUITE_END()
