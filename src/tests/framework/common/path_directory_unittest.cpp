#include <boost/test/unit_test.hpp>

#include <cr/common/throw.h>
#include <cr/framework/common/path_directory.h>

BOOST_AUTO_TEST_SUITE(name_directory)

BOOST_AUTO_TEST_CASE(exception)
{
    int categoryCode = -1;
    std::string message;
    try
    {
        CR_THROW(cr::framework::PathDirectoryException, cr::framework::PathDirectoryException::INVALID_PATH, "invalid path");
    }
    catch (cr::framework::PathDirectoryException& e)
    {
        categoryCode = e.getCategoryCode();
        message = e.getMessage();
    }
    BOOST_CHECK_EQUAL(categoryCode, cr::framework::PathDirectoryException::INVALID_PATH);
    BOOST_CHECK_EQUAL(message, "invalid path");
}

BOOST_AUTO_TEST_CASE(constructor)
{
    cr::framework::PathDirectory dict0;
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), 0);
}

BOOST_AUTO_TEST_CASE(copy_constructor)
{
    cr::framework::PathDirectory dict0;
    cr::framework::PathDirectory dict1 = dict0;
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), dict1.getRootChildrenCount());
}

BOOST_AUTO_TEST_CASE(move_constructor)
{
    cr::framework::PathDirectory dict0;
    std::size_t childrenCount0 = dict0.getRootChildrenCount();
    cr::framework::PathDirectory dict1 = std::move(dict0);
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), 0);
    BOOST_CHECK_EQUAL(dict1.getRootChildrenCount(), childrenCount0);
}

BOOST_AUTO_TEST_CASE(copy_assign)
{
    cr::framework::PathDirectory dict0;
    cr::framework::PathDirectory dict1;
    dict1 = dict0;
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), dict1.getRootChildrenCount());
}

BOOST_AUTO_TEST_CASE(move_assign)
{
    cr::framework::PathDirectory dict0;
    cr::framework::PathDirectory dict1;
    std::size_t childrenCount0 = dict0.getRootChildrenCount();
    dict1 = std::move(dict0);
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), 0);
    BOOST_CHECK_EQUAL(dict1.getRootChildrenCount(), childrenCount0);
}

BOOST_AUTO_TEST_CASE(swap)
{
    cr::framework::PathDirectory dict0;
    cr::framework::PathDirectory dict1;
    std::size_t childrenCount0 = dict0.getRootChildrenCount();
    std::size_t childrenCount1 = dict1.getRootChildrenCount();
    dict0.swap(dict1);
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), childrenCount1);
    BOOST_CHECK_EQUAL(dict1.getRootChildrenCount(), childrenCount0);
}

BOOST_AUTO_TEST_CASE(checkValidPath)
{
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath(""));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath(" "));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath(","));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("A"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("a"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("1"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("/"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("/ "));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("/,"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/A"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/a"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/1"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("//"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("/a "));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("/a,"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/aA"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/aa"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/a1"));
    BOOST_CHECK(!cr::framework::PathDirectory::checkValidPath("/a1/"));
    BOOST_CHECK(cr::framework::PathDirectory::checkValidPath("/a1/a"));
}

BOOST_AUTO_TEST_CASE(getRootChildrenCount)
{
    cr::framework::PathDirectory dict0;
    BOOST_CHECK_EQUAL(dict0.getRootChildrenCount(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

