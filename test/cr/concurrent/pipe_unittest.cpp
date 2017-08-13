#include <boost/test/unit_test.hpp>

#include <thread>

#include <cr/concurrent/pipe.h>
#include <cr/concurrent/spawn.h>

BOOST_AUTO_TEST_SUITE(Pipe)

BOOST_AUTO_TEST_CASE(pushAndPop)
{
    boost::asio::io_service ioService0;
    cr::concurrent::Pipe<int, std::mutex> pipe0(ioService0);
    boost::asio::io_service ioService1;
    cr::concurrent::Pipe<int, std::mutex> pipe1(ioService1);

    std::vector<int> elements0;
    cr::concurrent::spawn(ioService0, [&](cr::concurrent::Coroutine coro)
    {
        boost::system::error_code error;
        for (int element : {1, 2, 3, 4, 5})
        {
            pipe1.push(element);
            pipe0.pop(elements0, cr::concurrent::coro::async(coro, error));
        }
        auto elements = { 6, 7, 8, 9, 0 };
        pipe1.push(elements.begin(), elements.end());
        pipe0.pop(elements0, cr::concurrent::coro::async(coro, error));

        pipe1.cancel();
    });

    cr::concurrent::spawn(ioService1, [&](cr::concurrent::Coroutine coro)
    {
        boost::system::error_code error;
        while (!error)
        {
            std::vector<int> elements;
            pipe1.pop(elements, cr::concurrent::coro::async(coro, error));
            pipe0.push(elements.begin(), elements.end());
        }
    });

    std::thread thread0([&]
    {
        ioService0.run();
    });
    std::thread thread1([&]
    {
        ioService1.run();
    });
    thread0.join();
    thread1.join();
}

BOOST_AUTO_TEST_SUITE_END()