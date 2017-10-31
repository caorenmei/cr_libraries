#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/core/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_storage.h>
#include <cr/raft/raft_msg.pb.h>

BOOST_AUTO_TEST_SUITE(MemStorage)

auto makeEntry(std::uint64_t index, std::uint64_t term, std::string value)
{
    cr::raft::pb::Entry entry;
    entry.set_index(index);
    entry.set_term(term);
    entry.set_value(value);
    return entry;
}

BOOST_AUTO_TEST_CASE(append)
{
    cr::raft::MemStorage storage;
    storage.append({ makeEntry(1, 2, "0") });
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 1);
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 2);
}

BOOST_AUTO_TEST_CASE(remove)
{
    cr::raft::MemStorage storage;
    storage.append({ makeEntry(1, 2, "0") });
    storage.append({ makeEntry(2, 2, "0") });
    storage.append({ makeEntry(3, 3, "0") });
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 3);
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 3);

    storage.remove(3);
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 2);
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 2);

    storage.remove(1);
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 0);
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 0);

}

BOOST_AUTO_TEST_CASE(getEntries)
{
    cr::raft::MemStorage storage;
    storage.append({ makeEntry(1, 2, "0") });
    storage.append({ makeEntry(2, 2, "0") });
    storage.append({ makeEntry(3, 3, "0") });

    auto entries0 = storage.getEntries(1, 1, 1);
    BOOST_CHECK_EQUAL(entries0.size(), 1);

    auto entries1 = storage.getEntries(1, 2, 3);
    BOOST_CHECK_EQUAL(entries1.size(), 2);

    auto entries2 = storage.getEntries(1, 3, 2);
    BOOST_CHECK_EQUAL(entries2.size(), 2);
}

BOOST_AUTO_TEST_CASE(getTermByIndex)
{
    cr::raft::MemStorage storage;
    storage.append({ makeEntry(1, 2, "0") });
    storage.append({ makeEntry(2, 2, "0") });
    storage.append({ makeEntry(3, 3, "0") });

    BOOST_CHECK_EQUAL(storage.getTermByIndex(1), 2);
    BOOST_CHECK_EQUAL(storage.getTermByIndex(2), 2);
    BOOST_CHECK_EQUAL(storage.getTermByIndex(3), 3);
}

BOOST_AUTO_TEST_CASE(getLastIndex)
{
    cr::raft::MemStorage storage;
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 0);

    storage.append({ makeEntry(1, 2, "0") });
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 1);

    storage.append({ makeEntry(2, 2, "0") });
    storage.append({ makeEntry(3, 3, "0") });
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 3);
}

BOOST_AUTO_TEST_CASE(getLastTerm)
{
    cr::raft::MemStorage storage;
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 0);

    storage.append({ makeEntry(1, 2, "0") });
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 2);

    storage.append({ makeEntry(2, 2, "0") });
    storage.append({ makeEntry(3, 3, "0") });
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 3);
}


BOOST_AUTO_TEST_SUITE_END()
