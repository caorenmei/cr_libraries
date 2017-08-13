#include <boost/test/unit_test.hpp>

#include <iostream>

#include <boost/asio.hpp>

#include <cr/concurrent/spawn.h>
#include <boost/asio/system_timer.hpp>

BOOST_AUTO_TEST_SUITE(spawn)

BOOST_AUTO_TEST_CASE(spawn)
{
    boost::asio::io_service ioService;
    std::size_t loopCount = 0;
    cr::concurrent::spawn(ioService, [&](cr::concurrent::Coroutine coro)
    {
        boost::asio::system_timer timer(ioService);
        for (std::size_t i = 0; i != 10; ++i)
        {
            timer.expires_from_now(std::chrono::seconds(0));
            std::tuple<boost::system::error_code> r = timer.async_wait(cr::concurrent::coro::async(coro));
            BOOST_CHECK_EQUAL(std::get<0>(r), boost::system::error_code());
            timer.expires_from_now(std::chrono::seconds(0));
            timer.async_wait(cr::concurrent::coro::async(coro, std::get<0>(r)));
            BOOST_CHECK_EQUAL(std::get<0>(r), boost::system::error_code());
            loopCount = loopCount + 1;
        }
    });
    ioService.run();
    BOOST_CHECK_EQUAL(loopCount, 10);
}

BOOST_AUTO_TEST_SUITE_END()
