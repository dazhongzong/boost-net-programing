#include <iostream>
#include <boost/asio.hpp>
#include "../inc/Session.h"
#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include <csignal>
#include <thread>
#include <mutex>
//echo server   不能用于生产

// bool bstop = false;
// std::condition_variable cond_quit;
// std::mutex mutex_quit;


int main(){
    try
    {
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        signals.async_wait([&ioc](auto,auto/*注册信号的个数*/){
            ioc.stop();
        });

        CServer server(ioc,10086);
        ioc.run();

        //网络IO线程
        // //注册信号 及 处理函数
        // signal(SIGINT,sig_handler);
        // signal(SIGTERM,sig_handler);

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    
    }
    return 0;
}