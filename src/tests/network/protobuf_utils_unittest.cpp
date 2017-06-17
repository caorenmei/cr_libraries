#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/scope_guard.h>
#include <cr/network/protobuf_utils.h>

#include <tests/network/unittest.pb.h>

BOOST_AUTO_TEST_SUITE(protobuf_utils)

BOOST_AUTO_TEST_CASE(getProtobufMessageFromName)
{
    cr::unittest::HelloWorld helloWorld;
    auto message = cr::network::getProtobufMessageFromName(helloWorld.GetTypeName());
    CR_ON_SCOPE_EXIT([&] { delete message; });
    BOOST_CHECK(typeid(*message) == typeid(helloWorld));
}

BOOST_AUTO_TEST_SUITE_END()
