#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio.hpp>
#include <iostream>
using boost::asio::ip::tcp;
using boost::asio::awaitable;       //异步的等待
using boost::asio::co_spawn;        //启动协程的关键字
using boost::asio::detached;        //分离协程 协程独立启动
using boost::asio::use_awaitable;   //协程可以等待

namespace this_coro = boost::asio::this_coro;       //返回当前协程所使用的调度器  可以在协程中在开辟一个协程

boost::asio::awaitable<void> echo(tcp::socket socket)
{
    try
    {
        char data[1024](0);
        for(;;)
        {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data,1024),use_awaitable);        //变成协程可使用，并且阻塞的
            co_await async_write(socket,boost::asio::buffer(data,n),use_awaitable);
        }
    }
    catch(std::exception& e)
    {
        std::cout<<"echo exception is"<<e.what()<<std::endl;
    }
}

awaitable<void> listener()
{
    auto executor = co_await this_coro::executor;       //返回协程的调度器

    tcp::acceptor acceptor(executor,{boost::asio::ip::tcp::v4(),10086});
    for(;;)
    {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor,echo(std::move(socket)),detached);
    }
}

int main()
{
    try
    {
        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context,SIGINT,SIGTERM);
        signals.async_wait([&](auto,auto)
        {
            io_context.stop();
        });
        co_spawn(io_context,listener(),detached);               //启动一个协程，使用io_context的调度器，执行listerner任务，使协程独立运行

        io_context.run();
    }
    catch(std::exception& e)
    {
        std::cout<<"Exception is "<<e.what()<<std::endl;
    }


    return 0;
}