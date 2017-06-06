#include <boost/test/unit_test.hpp>

#include <cr/common/throw.h>

BOOST_AUTO_TEST_SUITE(cr_common_throw)

BOOST_AUTO_TEST_CASE(throw_macro)
{
    try
    {
        CR_THROW(cr::Error, "hello");
    }
    catch (cr::Error& e)
    {
        BOOST_CHECK_EQUAL(e.getMessage(), std::string("hello"));
        BOOST_CHECK_EQUAL(e.what(), std::string("hello"));
        BOOST_CHECK_EQUAL(e.getSourceLine(), 11);
    }
}

BOOST_AUTO_TEST_SUITE_END()