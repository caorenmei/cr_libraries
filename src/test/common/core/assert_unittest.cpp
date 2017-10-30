#include <boost/test/unit_test.hpp>

#include <cr/core/assert.h>

BOOST_AUTO_TEST_SUITE(cr_common_assert)

BOOST_AUTO_TEST_CASE(success)
{
    int a = 1;
    BOOST_CHECK_NO_THROW(CR_ASSERT(a == 1)(a));
}

BOOST_AUTO_TEST_CASE(failure)
{
    int a = 1;
    BOOST_CHECK_THROW(CR_ASSERT(a != 1)(a), cr::AssertError);
}

BOOST_AUTO_TEST_SUITE_END()
