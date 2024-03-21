#include <iostream>
#include <boost/asio.hpp>
#include "../inc/Session.h"
#include "../inc/CSession.h"
#include "../inc/CServer.h"
#include "../inc/AsioIOServicePool.h"
#include "../inc/AsioThreadPool.h"
#include <csignal>
#include <thread>
#include <mutex>


/*
该模式是 多个网络线程共享 一个io_context
连接请求也是 该 io_context 处理
网络IO则是多个线程用 同一个io_context处理
逻辑层 是单线程处理逻辑
*/

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main(){
    try
    {
        auto pool = AsioThreadPool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        // signals.add(); 注册信号
        signals.async_wait([&ioc,pool](boost::system::error_code e,int signalNumber){
            ioc.stop();
            pool->Stop();
            std::unique_lock<std::mutex> lock(mutex_quit);
            bstop = true;
            cond_quit.notify_one();
        });

        CServer server(pool->GetIOService(),10086);


        {
            std::unique_lock<std::mutex> lock(mutex_quit);
            
            while(!bstop)
            {
                cond_quit.wait(lock);
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    
    }
    return 0;
}