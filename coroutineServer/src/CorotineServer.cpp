#include <iostream>
#include "../inc/CServer.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "../inc/AsioIOServicePool.h"

int main()
{
    try
    {
        auto & pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context,SIGINT,SIGTERM);
        signals.async_wait([&pool,&io_context](auto,auto)
        {
            io_context.stop();
            pool.Stop();
        }
        );

        CServer server(io_context,10086);


        io_context.run();
    }
    catch(std::exception& e)
    {
        std::cout<<"Exception: "<<e.what()<<std::endl;
    }

    return 0;
}