#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/scope_guard.h>
#include <cr/protobuf/reflect.h>

#include "unittest.pb.h"

BOOST_AUTO_TEST_SUITE(reflect)

BOOST_AUTO_TEST_CASE(getMessageFromName)
{
    cr::unittest::HelloWorld helloWorld;
    auto message = cr::protobuf::getMessageFromName(helloWorld.GetTypeName());
    BOOST_REQUIRE_NE(message, nullptr);
    CR_SCOPE_EXIT([&] { delete message; });
    BOOST_CHECK(typeid(*message) == typeid(helloWorld));
}

BOOST_AUTO_TEST_SUITE_END()
