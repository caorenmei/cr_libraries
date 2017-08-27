#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/common/scope_guard.h>
#include <cr/network/protobuf_utils.h>

#include "unittest.pb.h"

BOOST_AUTO_TEST_SUITE(protobuf_utils)

BOOST_AUTO_TEST_CASE(parseAndSerializeProtobufMessage)
{
    cr::unittest::HelloWorld helloWorld0;
    helloWorld0.set_hello("hello");
    helloWorld0.set_world("world");

    cr::network::ByteBuffer buffer0;
    BOOST_CHECK(cr::network::serializeProtobufMessage(helloWorld0, buffer0));

    cr::unittest::HelloWorld helloWorld1;
    BOOST_CHECK(cr::network::parseProtobufMessage(helloWorld1, buffer0.data()));
    BOOST_CHECK_EQUAL(helloWorld0.hello(), helloWorld1.hello());
    BOOST_CHECK_EQUAL(helloWorld0.world(), helloWorld1.world());

    buffer0.prepare(10);
    buffer0.commit(10);
    BOOST_CHECK(cr::network::parseProtobufMessage(helloWorld1, buffer0.data(0, buffer0.getReadableBytes() - 10)));
    BOOST_CHECK_EQUAL(helloWorld0.hello(), helloWorld1.hello());
    BOOST_CHECK_EQUAL(helloWorld0.world(), helloWorld1.world());

    buffer0.clear();
    buffer0.shrink(100);
    buffer0.prepare(97);
    buffer0.commit(97);
    buffer0.consume(97);

    BOOST_CHECK(cr::network::serializeProtobufMessage(helloWorld0, buffer0));
    BOOST_CHECK(cr::network::parseProtobufMessage(helloWorld1, buffer0.data()));
    BOOST_CHECK_EQUAL(helloWorld0.hello(), helloWorld1.hello());
    BOOST_CHECK_EQUAL(helloWorld0.world(), helloWorld1.world());
}

BOOST_AUTO_TEST_SUITE_END()
