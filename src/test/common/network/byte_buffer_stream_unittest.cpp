#include <boost/test/unit_test.hpp>

#include <memory>

#include <cr/network/byte_buffer.h>
#include <cr/network/byte_buffer_stream.h>

#include "unittest.pb.h"

BOOST_AUTO_TEST_SUITE(protobuf_utils)

BOOST_AUTO_TEST_CASE(parseAndSerializeProtobufMessage)
{
    cr::unittest::HelloWorld helloWorld0;
    helloWorld0.set_hello("hello");
    helloWorld0.set_world("world");

    cr::network::ByteBuffer buffer0;
    cr::network::ByteBufferOutputStream stream0(buffer0);
    BOOST_CHECK(helloWorld0.SerializeToZeroCopyStream(&stream0));

    cr::unittest::HelloWorld helloWorld1;
    cr::network::ByteBufferInputStream stream1(buffer0.data());
    BOOST_CHECK(helloWorld1.ParseFromZeroCopyStream(&stream1));
    BOOST_CHECK_EQUAL(helloWorld0.hello(), helloWorld1.hello());
    BOOST_CHECK_EQUAL(helloWorld0.world(), helloWorld1.world());

    buffer0.prepare(10);
    buffer0.commit(10);
    cr::network::ByteBufferInputStream stream2(buffer0.data(0, buffer0.size() - 10));
    BOOST_CHECK(helloWorld1.ParseFromZeroCopyStream(&stream2));
    BOOST_CHECK_EQUAL(helloWorld0.hello(), helloWorld1.hello());
    BOOST_CHECK_EQUAL(helloWorld0.world(), helloWorld1.world());

    buffer0.clear();
    buffer0.shrink(100);
    buffer0.prepare(97);
    buffer0.commit(97);
    buffer0.consume(97);

    cr::network::ByteBufferOutputStream stream3(buffer0);
    BOOST_CHECK(helloWorld0.SerializeToZeroCopyStream(&stream3));
    cr::network::ByteBufferInputStream stream4(buffer0.data());
    BOOST_CHECK(helloWorld1.ParseFromZeroCopyStream(&stream4));
    BOOST_CHECK_EQUAL(helloWorld0.hello(), helloWorld1.hello());
    BOOST_CHECK_EQUAL(helloWorld0.world(), helloWorld1.world());
}

BOOST_AUTO_TEST_SUITE_END()
