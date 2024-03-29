#include "../inc/AsioIOServicePool.h"
#include <iostream>

AsioIOServicePool::AsioIOServicePool(std::size_t size):_ioServices(size),
    _works(size),
    _nextIOService(0)
    {
        //初始化work
        for(std::size_t i=0;i<size;i++)
        {
            _works[i] = WorkPtr(new Work(_ioServices[i]));
        }

        //遍历多个ioserivce,创建多个线程，每个线程内部启动ioservice
        for(std::size_t i =0;i<_ioServices.size();i++)
        {
            _threads.emplace_back([this,i]()
            {
                this->_ioServices[i].run();
            }
            );
        }
    }

AsioIOServicePool::~AsioIOServicePool()
{
    std::cout<<"AsioIOServicePool destruct"<<std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    auto & service = _ioServices[_nextIOService++];
    if(_nextIOService == _ioServices.size())
    {
        _nextIOService = 0;
    }

    return service;
}

void AsioIOServicePool::Stop()
{
    for(auto& work:_works)
    {
        work.reset();
    }

    //防止主线程结束 导致子线程挂掉
    for(auto& t:_threads)
    {
        t.join();
    }
}

AsioIOServicePool& AsioIOServicePool::GetInstance()
{
    //c++11 之后不存在线程安全问题
    static AsioIOServicePool instance(1);
    return instance;
}