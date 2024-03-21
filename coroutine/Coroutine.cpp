#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::awaitable;       //异步的等待
using boost::asio::co_spawn;        //启动协程的关键字
using boost::asio::detached;        //分离协程 协程独立启动
using boost::asio::use_awaitable;   //协程可以等待

namespace this_coro = boost::asio::this_coro;       //返回当前协程所使用的调度器  可以在协程中在开辟一个协程

int main()
{
    try
    {

    }
    catch(std::exception& e)
    {

    }


    return 0;
}