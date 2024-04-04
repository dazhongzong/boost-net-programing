#include "../inc/AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size)
:_ioServices(size),
_works(size),
_nextIOService(0)
{
    for(std::size_t i = 0;i<size; i++)
    {
        _works[i] = std::make_unique<Work>(_ioServices[i]);
    }

    //遍历多个ioservice,创建多个线程，每个线程内部启动ioservice;
    for(std::size_t i =0;i<_ioServices.size();i++)
    {
        _thread.emplace_back([this,i](){
            this->_ioServices[i].run();
        });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    std::cout<<"AsioIOServicePoll destruct "<<std::endl;
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
        work->get_io_context().stop();
        work.reset();
    }

    for(auto & t: _thread)
    {
        t.join();
    }
}