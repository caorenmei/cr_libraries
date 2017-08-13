#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_storage.h>
#include <cr/raft/raft_msg.pb.h>

BOOST_AUTO_TEST_SUITE(MemStorage)

auto makeEntry(std::uint64_t term, std::string value)
{
    cr::raft::pb::Entry entry;
    entry.set_term(term);
    entry.set_value(value);
    return entry;
}

BOOST_AUTO_TEST_CASE(append)
{
    cr::raft::MemStorage storage;
    BOOST_CHECK_THROW(storage.append(0, { makeEntry(1, "0") }), cr::raft::ArgumentException);
    BOOST_CHECK_NO_THROW(storage.append(1, { makeEntry(1, "1") }));
    BOOST_CHECK_THROW(storage.append(3, { makeEntry(1, "3") }), cr::raft::ArgumentException);
    BOOST_CHECK_NO_THROW(storage.append(2, { makeEntry(1, "2") }));

    auto getEntries = storage.getEntries(1, 2, std::numeric_limits<std::uint64_t>::max());
    BOOST_CHECK(cr::from(getEntries).map([](auto&& e) {return e.value(); }).equals(cr::from({ "1", "2" })));
}

BOOST_AUTO_TEST_CASE(remove)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_THROW(storage.remove(0), cr::raft::ArgumentException);

    storage.append(1, { makeEntry(1, "1") });
    storage.append(2, { makeEntry(1, "2") });
    storage.append(3, { makeEntry(1, "3") });
    BOOST_CHECK_THROW(storage.remove(0), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(storage.remove(4), cr::raft::ArgumentException);
    BOOST_CHECK_NO_THROW(storage.remove(3));
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 2);
    BOOST_CHECK_NO_THROW(storage.remove(1));
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 0);
}

BOOST_AUTO_TEST_CASE(getEntries)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_THROW(storage.getEntries(0, 0, std::numeric_limits<std::uint64_t>::max()), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(storage.getEntries(0, 1, std::numeric_limits<std::uint64_t>::max()), cr::raft::ArgumentException);

    storage.append(1, { makeEntry(1, "1") });
    BOOST_CHECK_THROW(storage.getEntries(0, 0, std::numeric_limits<std::uint64_t>::max()), cr::raft::ArgumentException);
    BOOST_CHECK_THROW(storage.getEntries(0, 1, std::numeric_limits<std::uint64_t>::max()), cr::raft::ArgumentException);
    BOOST_REQUIRE_NO_THROW(storage.getEntries(1, 1, std::numeric_limits<std::uint64_t>::max()));
    BOOST_CHECK_THROW(storage.getEntries(1, 2, std::numeric_limits<std::uint64_t>::max()), cr::raft::ArgumentException);
}

BOOST_AUTO_TEST_CASE(getTermByIndex)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_THROW(storage.getTermByIndex(0), cr::raft::ArgumentException);

    storage.append(1, { makeEntry(1, "1") });
    BOOST_CHECK_EQUAL(storage.getTermByIndex(1), 1);

    storage.append(2, { makeEntry(2, "2") });
    storage.append(3, { makeEntry(4, "3") });
    BOOST_CHECK_EQUAL(storage.getTermByIndex(2), 2);
    BOOST_CHECK_EQUAL(storage.getTermByIndex(3), 4); 
}

BOOST_AUTO_TEST_CASE(getLastIndex)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_EQUAL(storage.getLastIndex(), 0);

    storage.append(1, { makeEntry(1, "1") });
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 1);

    storage.append(2, { makeEntry(1, "2") });
    storage.append(3, { makeEntry(1, "3") });
    BOOST_CHECK_EQUAL(storage.getLastIndex(), 3);
}

BOOST_AUTO_TEST_CASE(getLastTerm)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_EQUAL(storage.getLastTerm(), 0);

    storage.append(1, { makeEntry(1, "1") });
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 1);

    storage.append(2, { makeEntry(2, "2") });
    storage.append(3, { makeEntry(4, "3") });
    BOOST_CHECK_EQUAL(storage.getLastTerm(), 4);
}


BOOST_AUTO_TEST_SUITE_END()
