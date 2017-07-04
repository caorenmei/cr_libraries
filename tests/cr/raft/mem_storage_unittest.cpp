#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/streams.h>
#include <cr/raft/exception.h>
#include <cr/raft/mem_storage.h>

BOOST_AUTO_TEST_SUITE(MemStorage)

BOOST_AUTO_TEST_CASE(append)
{
    cr::raft::MemStorage storage;
    BOOST_CHECK_THROW(storage.append({ 0, 1, "0" }), cr::raft::StoreException);
    BOOST_CHECK_NO_THROW(storage.append({ 1, 1, "1" }));
    BOOST_CHECK_THROW(storage.append({ 3, 1, "3" }), cr::raft::StoreException);
    BOOST_CHECK_NO_THROW(storage.append({ 2, 1, "2" }));

    auto entries = storage.entries(1, 2);
    BOOST_CHECK(cr::from(entries).map([](auto&& e) {return e.getValue(); }).equals(cr::from({ "1", "2" })));
}

BOOST_AUTO_TEST_CASE(remove)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_THROW(storage.remove(0), cr::raft::StoreException);

    storage.append({ 1, 1, "1" });
    storage.append({ 2, 1, "2" });
    storage.append({ 3, 1, "3" });
    BOOST_CHECK_THROW(storage.remove(0), cr::raft::StoreException);
    BOOST_CHECK_THROW(storage.remove(4), cr::raft::StoreException);
    BOOST_CHECK_NO_THROW(storage.remove(3));
    BOOST_CHECK_EQUAL(storage.lastIndex(), 2);
    BOOST_CHECK_NO_THROW(storage.remove(1));
    BOOST_CHECK_EQUAL(storage.lastIndex(), 0);
}

BOOST_AUTO_TEST_CASE(entries)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_THROW(storage.entries(0, 0), cr::raft::StoreException);
    BOOST_CHECK_THROW(storage.entries(0, 1), cr::raft::StoreException);

    storage.append({ 1, 1, "1" });
    BOOST_CHECK_THROW(storage.entries(0, 0), cr::raft::StoreException);
    BOOST_CHECK_THROW(storage.entries(0, 1), cr::raft::StoreException);
    BOOST_REQUIRE_NO_THROW(storage.entries(1, 1));
    BOOST_CHECK_THROW(storage.entries(1, 2), cr::raft::StoreException);
}

BOOST_AUTO_TEST_CASE(term)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_THROW(storage.term(0), cr::raft::StoreException);

    storage.append({ 1, 1, "1" });
    BOOST_CHECK_EQUAL(storage.term(1), 1);

    storage.append({ 2, 2, "2" });
    storage.append({ 3, 4, "3" });
    BOOST_CHECK_EQUAL(storage.term(2), 2);
    BOOST_CHECK_EQUAL(storage.term(3), 4); 
}

BOOST_AUTO_TEST_CASE(lastIndex)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_EQUAL(storage.lastIndex(), 0);

    storage.append({ 1, 1, "1" });
    BOOST_CHECK_EQUAL(storage.lastIndex(), 1);

    storage.append({ 2, 1, "2" });
    storage.append({ 3, 1, "3" });
    BOOST_CHECK_EQUAL(storage.lastIndex(), 3);
}

BOOST_AUTO_TEST_CASE(lastTerm)
{
    cr::raft::MemStorage storage;

    BOOST_CHECK_EQUAL(storage.lastTerm(), 0);

    storage.append({ 1, 1, "1" });
    BOOST_CHECK_EQUAL(storage.lastTerm(), 1);

    storage.append({ 2, 2, "2" });
    storage.append({ 3, 4, "3" });
    BOOST_CHECK_EQUAL(storage.lastTerm(), 4);
}


BOOST_AUTO_TEST_SUITE_END()
