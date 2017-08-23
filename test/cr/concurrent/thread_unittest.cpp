#include <boost/test/unit_test.hpp>

#include <atomic>

#include <cr/concurrent/thread.h>

BOOST_AUTO_TEST_SUITE(Thread)

BOOST_AUTO_TEST_CASE(Thread)
{
    auto ioService0 = std::make_shared<boost::asio::io_service>();
    cr::concurrent::Thread t0(ioService0);
    int value0 = 0;
    t0.post([&]
    {
        value0 = 1;
    });
    t0.post([&]
    {
        t0.stop();
    });
    ioService0->run();
    t0.join();
    BOOST_CHECK_EQUAL(value0, 1);

    cr::concurrent::Thread t1;
    std::atomic<int> value1 = 0;
    t1.post([&]
    {
        value1 = 1;
    });
    t1.post([&]
    {
        t1.stop();
    });
    t1.join();
    BOOST_CHECK_EQUAL(value1, 1);
}

BOOST_AUTO_TEST_SUITE_END()