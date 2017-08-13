#include <boost/test/unit_test.hpp>

#include <exception>

#include <cr/common/throw.h>

BOOST_AUTO_TEST_SUITE(cr_common_throw)

BOOST_AUTO_TEST_CASE(throw_macro)
{
    std::exception_ptr eptr;
    try
    {
        CR_THROW(cr::Error, "hello");
    }
    catch (...)
    {
        eptr = std::current_exception();
    }
    BOOST_REQUIRE(eptr !=  nullptr);
    BOOST_REQUIRE_THROW(std::rethrow_exception(eptr), cr::Error);
    try
    {
        std::rethrow_exception(eptr);
    }
    catch (cr::Error& e)
    {
        BOOST_CHECK_EQUAL(e.getMessage(), std::string("hello"));
        BOOST_CHECK_EQUAL(e.what(), std::string("hello"));
        BOOST_CHECK_EQUAL(e.getSourceLine(), 14);
    }
}

BOOST_AUTO_TEST_CASE(if_throw_macro)
{
    std::exception_ptr eptr;
    try
    {
        CR_IF_THROW(1 != 1, cr::Error, "hello");
    }
    catch (...)
    {
        eptr = std::current_exception();
    }
    BOOST_REQUIRE(eptr == nullptr);

    try
    {
        CR_IF_THROW(1 == 1, cr::Error, "hello");
    }
    catch (...)
    {
        eptr = std::current_exception();
    }
    BOOST_REQUIRE(eptr != nullptr);
}

BOOST_AUTO_TEST_SUITE_END()