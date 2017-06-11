#include <boost/test/unit_test.hpp>

#include <cr/network/byte_buffer.h>

BOOST_AUTO_TEST_SUITE(cr_network_byte_buffer)

template <typename Buffers>
auto bufferToString(const Buffers& buffers)
{
    std::string data(boost::asio::buffer_size(buffers), '\0');
    boost::asio::buffer_copy(boost::asio::buffer(&data[0], data.size()), buffers);
    return data;
}

BOOST_AUTO_TEST_CASE(capacity)
{
    cr::network::ByteBuffer buffer1(0);
    BOOST_CHECK_EQUAL(buffer1.getCapacity(), 0);
    cr::network::ByteBuffer buffer2(100);
    BOOST_CHECK_EQUAL(buffer2.getCapacity(), 100);
}

BOOST_AUTO_TEST_CASE(getReadableBytes)
{
    std::string data = "1234567890";

    cr::network::ByteBuffer buffer0(0);
    buffer0.prepare(data.size());
    buffer0.commit(data.size());
    BOOST_CHECK_EQUAL(buffer0.getReadableBytes(), data.size());

    cr::network::ByteBuffer buffer1(data.size());
    buffer1.prepare(data.size());
    buffer1.commit(data.size());
    BOOST_CHECK_EQUAL(buffer1.getReadableBytes(), data.size());

    cr::network::ByteBuffer buffer2(100);
    buffer2.prepare(data.size());
    buffer2.commit(data.size());
    BOOST_CHECK_EQUAL(buffer2.getReadableBytes(), data.size());
}

BOOST_AUTO_TEST_CASE(prepare_data_commit)
{
    std::string data = "1234567890";

    cr::network::ByteBuffer buffer0(0);
    boost::asio::buffer_copy(buffer0.prepare(data.size()), boost::asio::buffer(data));
    buffer0.commit(data.size());
    BOOST_CHECK_EQUAL(data, bufferToString(buffer0.data()));

    cr::network::ByteBuffer buffer1(data.size());
    boost::asio::buffer_copy(buffer1.prepare(data.size()), boost::asio::buffer(data));
    buffer1.commit(data.size());
    BOOST_CHECK_EQUAL(data, bufferToString(buffer1.data()));

    cr::network::ByteBuffer buffer2(100);
    boost::asio::buffer_copy(buffer2.prepare(100), boost::asio::buffer(data));
    buffer2.commit(data.size());
    BOOST_CHECK_EQUAL(data, bufferToString(buffer2.data()));

    cr::network::ByteBuffer buffer3(10);
    buffer3.prepare(5);
    buffer3.commit(5);
    buffer3.consume(5);
    boost::asio::buffer_copy(buffer3.prepare(data.size()), boost::asio::buffer(data));
    buffer3.commit(data.size());
    BOOST_CHECK_EQUAL(data, bufferToString(buffer3.data()));
}

BOOST_AUTO_TEST_CASE(consume)
{
    cr::network::ByteBuffer buffer0(0);
    buffer0.prepare(5);
    buffer0.commit(5);
    buffer0.consume(5);
    BOOST_CHECK_EQUAL(buffer0.getReadableBytes(), 0);

    cr::network::ByteBuffer buffer1(10);
    buffer1.prepare(10);
    buffer1.commit(10);
    buffer1.consume(5);
    BOOST_CHECK_EQUAL(buffer1.getReadableBytes(), 5);
    buffer1.consume(5);
    BOOST_CHECK_EQUAL(buffer1.getReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(set_get)
{
    std::string data0 = "1234567890";
    std::string data1 = "12345";

    std::string dest0(data0.size(), '0');
    std::string dest1(data1.size(), '0');

    cr::network::ByteBuffer buffer0(0);
    buffer0.prepare(10);
    buffer0.commit(10);

    buffer0.set(0, data0.data(), data0.size());
    buffer0.get(0, &dest0[0], dest0.size());
    BOOST_CHECK_EQUAL(data0, dest0);

    buffer0.set(0, data1.data(), data1.size());
    buffer0.get(0, &dest1[0], dest1.size());
    BOOST_CHECK_EQUAL(data1, dest1);

    buffer0.set(3, data1.data(), data1.size());
    buffer0.get(3, &dest1[0], dest1.size());
    BOOST_CHECK_EQUAL(data1, dest1);

    buffer0.consume(5);
    buffer0.prepare(5);
    buffer0.commit(5);

    buffer0.set(0, data0.data(), data0.size());
    buffer0.get(0, &dest0[0], dest0.size());
    BOOST_CHECK_EQUAL(data0, dest0);

    buffer0.set(0, data1.data(), data1.size());
    buffer0.get(0, &dest1[0], dest1.size());
    BOOST_CHECK_EQUAL(data1, dest1);

    buffer0.set(3, data1.data(), data1.size());
    buffer0.get(3, &dest1[0], dest1.size());
    BOOST_CHECK_EQUAL(data1, dest1); 
}

BOOST_AUTO_TEST_CASE(swap)
{
    std::string data0 = "1234567890";
    std::string data1 = "abcdefghij";

    cr::network::ByteBuffer buffer0(0);
    boost::asio::buffer_copy(buffer0.prepare(data0.size()), boost::asio::buffer(data0));
    buffer0.commit(data0.size());

    cr::network::ByteBuffer buffer1(10);
    boost::asio::buffer_copy(buffer1.prepare(data1.size()), boost::asio::buffer(data1));
    buffer1.commit(data1.size());

    buffer1.swap(buffer0);
    BOOST_CHECK_EQUAL(data0, bufferToString(buffer1.data()));
    BOOST_CHECK_EQUAL(data1, bufferToString(buffer0.data()));

    cr::network::ByteBuffer buffer2(30);
    buffer2.prepare(10);
    buffer2.commit(10);
    buffer2.consume(10);
    boost::asio::buffer_copy(buffer2.prepare(data0.size()), boost::asio::buffer(data0));
    buffer2.commit(data0.size());

    cr::network::ByteBuffer buffer3(10);
    boost::asio::buffer_copy(buffer3.prepare(data1.size()), boost::asio::buffer(data1));
    buffer3.commit(data1.size());

    buffer3.swap(buffer2);
    BOOST_CHECK_EQUAL(data0, bufferToString(buffer3.data()));
    BOOST_CHECK_EQUAL(data1, bufferToString(buffer2.data()));
}

BOOST_AUTO_TEST_CASE(move)
{
    std::string data0 = "1234567890";

    cr::network::ByteBuffer buffer0(0);
    boost::asio::buffer_copy(buffer0.prepare(data0.size()), boost::asio::buffer(data0));
    buffer0.commit(data0.size());

    cr::network::ByteBuffer buffer1 = std::move(buffer0);
    BOOST_CHECK_EQUAL(data0, bufferToString(buffer1.data()));
    BOOST_CHECK_EQUAL(buffer0.getReadableBytes(), 0);
}

BOOST_AUTO_TEST_CASE(clear)
{
    cr::network::ByteBuffer buffer0(0);
    buffer0.prepare(5);
    buffer0.commit(5);
    buffer0.consume(2);
    buffer0.clear();
    BOOST_CHECK_EQUAL(buffer0.getReadableBytes(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
