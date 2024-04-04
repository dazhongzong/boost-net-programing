#pragma once
#include <memory>
#include <mutex>
#include <iostream>

template<class T>
class Singleton
{
private:
    Singleton() = delete;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static std::shared_ptr<T> _instance;
public:

    ~Singleton()
    {
        std::cout<<"this is singleton destructer"<<std::endl;
    }
    
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_flag;
        std::call_once(s_flag,[&]()
        {
            _instance = std::shared_ptr<T>(new T);
        });

        return _instance;
    }
};

template <class T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;