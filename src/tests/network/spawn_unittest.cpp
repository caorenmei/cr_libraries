#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>

#include <cr/network/spawn.h>

BOOST_AUTO_TEST_SUITE(cr_network_spawn)

BOOST_AUTO_TEST_CASE(spawn)
{
    boost::asio::io_service ioService;
    cr::network::spawn(ioService, [&](cr::network::Coroutine coro)
    {
        for (std::size_t i = 0; i != 10; ++i)
        {
            ioService.post([&]
            {
                coro.resume();
            });
            coro.yield();
        }
    });
    ioService.run();
}

BOOST_AUTO_TEST_SUITE_END()
