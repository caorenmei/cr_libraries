
#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/timer/timer.hpp>

#include <cr/common/streams.h>
#include <cr/concurrent/pipe.h>
#include <cr/concurrent/spawn.h>

void multiThread()
{
    boost::asio::io_service ioService1;
    boost::asio::io_service ioService2;

    cr::concurrent::Pipe<int, std::mutex> pipe1(ioService1);
    cr::concurrent::Pipe<int, std::mutex> pipe2(ioService2);

    // 发起任务，等待任务结果
    cr::concurrent::spawn(ioService1, [&](cr::concurrent::Coroutine coro)
    {
        boost::system::error_code ec;
        for (int i = 0; i != 10 * 10000; ++i)
        {
            pipe2.push(i);
            pipe1.pop(cr::concurrent::coro::async(coro, ec));
        }
        pipe2.shutdown();
    });

    // 处理任务，回执结果
    cr::concurrent::spawn(ioService2, [&](cr::concurrent::Coroutine coro)
    {
        boost::system::error_code ec;
        while (!ec)
        {
            int element = 0;
            std::tie(ec, element) = pipe2.pop(cr::concurrent::coro::async(coro));
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

    cr::concurrent::Pipe<int> pipe1(ioService);
    cr::concurrent::Pipe<int> pipe2(ioService);

    // 发起任务，等待任务结果
    cr::concurrent::spawn(strand, [&](cr::concurrent::Coroutine coro)
    {
        boost::system::error_code ec;
        for (int i = 0; i != 100 * 10000; ++i)
        {
            pipe2.push(i);
            pipe1.pop(cr::concurrent::coro::async(coro, ec));
        }
        pipe2.shutdown();
    });

    // 处理任务，回执结果
    cr::concurrent::spawn(strand, [&](cr::concurrent::Coroutine coro)
    {
        boost::system::error_code ec;
        while (!ec)
        {
            int element = 0;
            std::tie(ec, element) = pipe2.pop(cr::concurrent::coro::async(coro));
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