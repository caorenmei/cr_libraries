#include <boost/test/unit_test.hpp>

#include <exception>

#include <cr/common/exception.h>

BOOST_AUTO_TEST_SUITE(cr_common_exception)

BOOST_AUTO_TEST_CASE(constructr)
{
    std::exception_ptr eptr;
    try
    {
        cr::Exception e("hello");
        e.setSourceLine(100);
        e.setSourceName("hello.cpp");
        throw e;
    }
    catch (...)
    {
        eptr = std::current_exception();
    }
    BOOST_REQUIRE(eptr != nullptr);
    BOOST_REQUIRE_THROW(std::rethrow_exception(eptr), cr::Exception);
    try
    {
        std::rethrow_exception(eptr);
    }
    catch (cr::Exception& e)
    {
        BOOST_CHECK_EQUAL(e.getMessage(), std::string("hello"));
        BOOST_CHECK_EQUAL(e.what(), std::string("hello"));
        BOOST_CHECK_EQUAL(e.getSourceLine(), 100);
        BOOST_CHECK_EQUAL(e.getSourceName(), std::string("hello.cpp"));
    }
}

BOOST_AUTO_TEST_SUITE_END()