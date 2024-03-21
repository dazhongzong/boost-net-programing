#pragma once
#include "Singleton.h"
#include <boost/asio.hpp>
#include <thread>
#include <vector>

class AsioThreadPool: public Singleton<AsioThreadPool>
{
    friend Singleton<AsioThreadPool>;
public:
    ~AsioThreadPool(){}
    AsioThreadPool& operator=(const AsioThreadPool&) = delete;//可以不加 因为父类不可拷贝构造、拷贝赋值
    AsioThreadPool(const AsioThreadPool&) =delete;
    boost::asio::io_context& GetIOService();
    void Stop();
private:
    AsioThreadPool(std::size_t size = std::thread::hardware_concurrency());

    boost::asio::io_context _service;
    std::unique_ptr<boost::asio::io_context::work> _work;

    std::vector<std::thread> _threads;
};