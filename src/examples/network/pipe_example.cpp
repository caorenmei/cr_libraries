
#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/timer/timer.hpp>

#include <cr/common/streams.h>
#include <cr/network/pipe.h>
#include <cr/network/spawn.h>

void multiThread()
{
    boost::asio::io_service ioService1;
    boost::asio::io_service ioService2;

    cr::network::Pipe<int, std::mutex> pipe1(ioService1);
    cr::network::Pipe<int, std::mutex> pipe2(ioService2);

    // 发起任务，等待任务结果
    cr::network::spawn(ioService1, [&](cr::network::Coroutine coro)
    {
        boost::system::error_code ec;
        for (int i = 0; i != 10 * 10000; ++i)
        {
            pipe2.push(i);
            pipe1.pop(cr::network::coro::async(coro, ec));
        }
        pipe2.cancel();
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
}

void singleThread()
{
    boost::asio::io_service ioService;
    boost::asio::io_service::strand strand(ioService);

    cr::network::Pipe<int> pipe1(ioService);
    cr::network::Pipe<int> pipe2(ioService);

    // 发起任务，等待任务结果
    cr::network::spawn(strand, [&](cr::network::Coroutine coro)
    {
        boost::system::error_code ec;
        for (int i = 0; i != 100 * 10000; ++i)
        {
            pipe2.push(i);
            pipe1.pop(cr::network::coro::async(coro, ec));
        }
        strand.post([&] {pipe2.cancel(); });
    });

    // 处理任务，回执结果
    cr::network::spawn(strand, [&](cr::network::Coroutine coro)
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

    std::thread t1([&] { ioService.run(); });
    t1.join();
}

int main(int argc, char* argv[])
{
    {
        std::cout << "multi thread: " << std::endl;
        boost::timer::auto_cpu_timer t;
        multiThread();
    }
    {
        std::cout << "\nsingle thread: " << std::endl;
        boost::timer::auto_cpu_timer t;
        singleThread();
    }
}