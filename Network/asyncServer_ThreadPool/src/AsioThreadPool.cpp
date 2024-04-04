#include "../inc/AsioThreadPool.h"

AsioThreadPool::AsioThreadPool(std::size_t size)
:_work(new boost::asio::io_context::work(_service))
{
    for(std::size_t i = 0;i<size ;++i)
    {
        _threads.emplace_back([this]()
            {
                this->_service.run();
            }
            );
    }
}

boost::asio::io_context& AsioThreadPool::GetIOService()
{
    return _service;
}

void AsioThreadPool::Stop()
{
    _service.stop();
    _work.reset();

    for(auto& t:_threads)
    {
        t.join();
    }
}

