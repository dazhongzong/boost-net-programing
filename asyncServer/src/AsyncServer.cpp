#include <iostream>
#include <boost/asio.hpp>
#include "../inc/Session.h"
#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include "../inc/AsioIOServicePool.h"
#include <csignal>
#include <thread>
#include <mutex>
//echo server   不能用于生产

// bool bstop = false;
// std::condition_variable cond_quit;
// std::mutex mutex_quit;oo


int main(){
    try
    {
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        // signals.add(); 注册信号
        signals.async_wait([&ioc](boost::system::error_code e,int signalNumber){
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