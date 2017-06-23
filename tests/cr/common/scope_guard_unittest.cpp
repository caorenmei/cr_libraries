#include <boost/test/unit_test.hpp>

#include <cr/common/scope_guard.h>

BOOST_AUTO_TEST_SUITE(cr_common_scope_guard)

BOOST_AUTO_TEST_CASE(on_scope_guard)
{
    int a = 1;
    {
        CR_SCOPE_EXIT([&] { a = 0; });
    }
    BOOST_CHECK_EQUAL(a, 0);
}

BOOST_AUTO_TEST_CASE(dismiss)
{
    int a = 1;
    {
        cr::ScopeGuard onExit([&] {a = 0; });
        onExit.dismiss();
    }
    BOOST_CHECK_EQUAL(a, 1);
}

BOOST_AUTO_TEST_CASE(move_constructor)
{
    int a = 1;
    {
        cr::ScopeGuard onExit1([&] {a -= 1; });
        cr::ScopeGuard onExit2(std::move(onExit1));
    }
    BOOST_CHECK_EQUAL(a, 0);
}

BOOST_AUTO_TEST_SUITE_END()