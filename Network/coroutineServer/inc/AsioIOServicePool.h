#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>
class AsioIOServicePool
{
public:
    using IOService = boost::asio::io_context;
    using Work      = boost::asio::io_context::work;
    using WorkPtr   = std::shared_ptr<Work>;
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) =delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
    boost::asio::io_context& GetIOService();
    void Stop();
    static AsioIOServicePool& GetInstance();        //返回单例实例
private:
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOService;
};