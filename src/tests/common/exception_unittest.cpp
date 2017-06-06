#include <boost/test/unit_test.hpp>

#include <cr/common/exception.h>

BOOST_AUTO_TEST_SUITE(cr_common_exception)

BOOST_AUTO_TEST_CASE(constructr)
{
    try
    {
        cr::Exception e("hello");
        e.setSourceLine(100);
        e.setSourceName("hello.cpp");
        throw e;
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