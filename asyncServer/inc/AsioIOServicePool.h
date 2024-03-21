#pragma once

#include "Singleton.h"
#include <boost/asio.hpp>
#include <vector>

class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work;     //防止io_context 没有被注册的时候退出
    using WorkPtr = std::unique_ptr<Work>;
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
    //使用round-robin 的方式返回一个io_context
    boost::asio::io_context& GetIOService();
    void Stop();
private:
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOService> _ioServices; 
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _thread;

    std::size_t _nextIOService;
};