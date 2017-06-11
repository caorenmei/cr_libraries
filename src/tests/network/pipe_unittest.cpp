#include <boost/test/unit_test.hpp>

#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <cr/common/streams.h>
#include <cr/network/pipe.h>
#include <cr/network/spawn.h>

BOOST_AUTO_TEST_SUITE(cr_network_pipe)

BOOST_AUTO_TEST_CASE(pipe)
{
    boost::asio::io_service ioService1;
    boost::asio::io_service ioService2;

    cr::network::Pipe<int, std::mutex> pipe1(ioService1);
    cr::network::Pipe<int, std::mutex> pipe2(ioService2);

    std::vector<int> ivec;

    // 发起任务，等待任务结果
    cr::network::spawn(ioService1, [&](cr::network::Coroutine coro)
    {
        for (int element1 : cr::generate(0, 1).limit(10))
        {
            pipe2.push(element1);
            boost::system::error_code ec;
            int element2 = 0;
            pipe1.pop(cr::network::coro::async(coro, ec, element2));
            ivec.push_back(element2);
        }
        pipe2.shutdown();
    });

    // 处理任务，回执结果
    cr::network::spawn(ioService2, [&](cr::network::Coroutine coro)
    {
        boost::system::error_code ec;
        while (!ec)
        {
            int element = 0;
            std::tie(ec, element) = pipe2.pop(cr::network::coro::async(coro));
            if (!ec)
            {
                pipe1.push(element * 10);
            }
        }
    });
    
    std::thread t1([&] { ioService1.run(); });
    std::thread t2([&] { ioService2.run(); });
    t1.join();
    t2.join();

    BOOST_CHECK(cr::from(ivec).equals(cr::generate(0, 10).limit(10)));
}

BOOST_AUTO_TEST_SUITE_END()
