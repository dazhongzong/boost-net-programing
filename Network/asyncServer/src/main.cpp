#include <iostream>
#include <boost/asio.hpp>
#include "../inc/Session.h"
#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include "../inc/AsioIOServicePool.h"
#include <csignal>
#include <thread>
#include <mutex>

/*
该模式 一个io_context处理连接请求,
多个线程多个io_context 处理网络IO事件
一个逻辑线程 处理 逻辑层
*/

int main(){
    try
    {
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        // signals.add(); 注册信号
        signals.async_wait([&ioc,pool](boost::system::error_code e,int signalNumber){
            ioc.stop();
            pool->Stop();
        });

        CServer server(ioc,10086);
        ioc.run();

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    
    }
    return 0;
}