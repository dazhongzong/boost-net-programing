// day02-lockmutex.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <mutex>
#include <thread>
#include <stack>
#include <memory>
std::mutex mtx1;
int shared_data = 100;

void use_lock()
{
    while (true)
    {
        mtx1.lock();
        shared_data++;
        std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
        std::cout << "shared data is " << shared_data << std::endl;
        mtx1.unlock();
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));//释放cpu资源
    }
}

void test_lock()
{
    std::thread t1(use_lock);

    std::thread t2([]() {
        while (true)
        {
            {
                std::lock_guard<std::mutex> lock(mtx1);
                shared_data--;
                std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
                std::cout << "shared data is " << shared_data << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));//释放cpu资源
        }
        });


    t1.join();
    t2.join();
}

template<typename T>
class threadsafe_stack1
{
private:
    std::stack<T> data;
    mutable std::mutex m;           //
public:
    threadsafe_stack1(){}
    threadsafe_stack1(const threadsafe_stack1& rhs)
    {
        {
            std::lock_guard(rhs.m);
            data = rhs.data;
        }
    }
    threadsafe_stack1& operator=(const threadsafe_stack1&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    //问题代码
    T pop()
    {
        std::lock_guard<std::mutex> lock(m);
        auto element = data.top();
        data.pop();
        return element;
    }

    //危险 可能在使用该结果前 别的线程已经操作了导致实际结果发生了改变
    bool empty()const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

void test_threadsafe_stack1()
{
    threadsafe_stack1<int> safe_stack;
    safe_stack.push(1);

    std::thread t1([&safe_stack]() {
        if (!safe_stack.empty())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            safe_stack.pop();
        }
        });

    std::thread t2([&safe_stack]() {
        if (!safe_stack.empty())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            safe_stack.pop();
        }
        });


    t1.join();
    t2.join();
}

struct empty_stack : std::exception
{
    const char* what() throw();
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& rhs)
    {
        {
            std::lock_guard(rhs.m);
            data = rhs.data;
        }
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    //问题代码   可能会导致内存中栈空间 不足
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty())
        {
            //throw empty_stack();
            return nullptr;
        }
        auto element = std::make_shared<T>(data.top());
        data.pop();
        return element;
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty())
        {
            throw empty_stack();
        }
        value = std::move(data.top());
        data.pop();
    }

    //危险 可能在使用该结果前 别的线程已经操作了导致实际结果发生了改变
    bool empty()const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

std::mutex  t_lock1;
std::mutex  t_lock2;
int m_1 = 0;
int m_2 = 1;

void dead_lock1() {
    while (true) {
        std::cout << "dead_lock1 begin " << std::endl;
        t_lock1.lock();
        m_1 = 1024;
        t_lock2.lock();
        m_2 = 2048;
        t_lock2.unlock();
        t_lock1.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "dead_lock2 end " << std::endl;
    }
}

void dead_lock2() {
    while (true) {
        std::cout << "dead_lock2 begin " << std::endl;
        t_lock2.lock();
        m_2 = 2048;
        t_lock1.lock();
        m_1 = 1024;
        t_lock1.unlock();
        t_lock2.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "dead_lock2 end " << std::endl;
    }
}

void test_dead_lock() {
    std::thread t1(dead_lock1);
    std::thread t2(dead_lock2);
    t1.join();
    t2.join();
}

//加锁和解锁作为原子操作解耦合，各自只管理自己的功能
void atomic_lock1() {
    std::cout << "lock1 begin lock" << std::endl;
    t_lock1.lock();
    m_1 = 1024;
    t_lock1.unlock();
    std::cout << "lock1 end lock" << std::endl;
}

void atomic_lock2() {
    std::cout << "lock2 begin lock" << std::endl;
    t_lock2.lock();
    m_2 = 2048;
    t_lock2.unlock();
    std::cout << "lock2 end lock" << std::endl;
}

void safe_lock1() {
    while (true) {
        atomic_lock1();
        atomic_lock2();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void safe_lock2() {
    while (true) {
        atomic_lock2();
        atomic_lock1();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void test_safe_lock() {
    std::thread t1(safe_lock1);
    std::thread t2(safe_lock2);
    t1.join();
    t2.join();
}

//对于要使用两个互斥量，可以同时加锁
//如不同时加锁可能会死锁

//假设这是一个很复杂的数据结构，假设我们不建议执行拷贝构造
class some_big_object
{
public:
    some_big_object(int data) :_data(data) {}
    //拷贝构造
    some_big_object(const some_big_object& rhs):_data(rhs._data)
    {
    }
    //移动构造
    some_big_object(some_big_object&& rhs) noexcept:_data(std::move(rhs._data))
    {
    }
    //重载拷贝赋值运算符
    some_big_object& operator=(const some_big_object& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        _data = rhs._data;
        return *this;
    }
    //重载移动赋值
    some_big_object& operator=(some_big_object&& rhs)
    {
        _data = std::move(rhs._data);
        return *this;
    }

    //交换数据
    friend void swap(some_big_object& lhs, some_big_object& rhs)
    {
        some_big_object temp = std::move(lhs);
        lhs = std::move(rhs);
        rhs = std::move(temp);

    }

    //重载
    friend std::ostream& operator<<(std::ostream& os, const some_big_object& rhs)
    {
        os << rhs._data;
        return os;
    }
private:
    int _data;
};

//假设这个一个结构，包含了复杂的成员对象和一个互斥量
class big_object_mgr
{
public:
    big_object_mgr(int data = 0)
        :_obj(data)
    {

    }
    void printinfo()
    {
        std::cout << "current obj data is " << _obj << std::endl;
    }

    friend void danger_swap(big_object_mgr& objm1, big_object_mgr& objm2);
    friend void safe_swap(big_object_mgr& objm1, big_object_mgr& objm2);
    friend void safe_swap_scope(big_object_mgr& objm1, big_object_mgr& objm2);

private:
    std::mutex _mtx;
    some_big_object _obj;
};

void danger_swap(big_object_mgr& objm1, big_object_mgr& objm2)
{
    std::cout << "thread [" << std::this_thread::get_id() << "] begin" << std::endl;
    if (&objm1 == &objm2)
    {
        return;
    }
    
    std::lock_guard < std::mutex > guard1(objm1._mtx);
    //此处为了让死锁必现，先睡一会
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::lock_guard<std::mutex> guard2(objm2._mtx);
    swap(objm1._obj, objm2._obj);

    std::cout << "thread [" << std::this_thread::get_id() << "] end" << std::endl;
}
void test_danger_swap()
{
    big_object_mgr objm1(5);
    big_object_mgr objm2(100);
    std::thread t1(danger_swap, std::ref(objm1), std::ref(objm2));
    std::thread t2(danger_swap, std::ref(objm2), std::ref(objm1));
    t1.join();
    t2.join();
    
    objm1.printinfo();
    objm2.printinfo();
}

void safe_swap(big_object_mgr& objm1, big_object_mgr& objm2)
{
    std::cout << "thread [" << std::this_thread::get_id() << "] begin" << std::endl;
    if (&objm1 == &objm2)
    {
        return;
    }
    //同时加锁
    std::lock(objm1._mtx, objm2._mtx);
    //领养锁管理互斥量解锁
    std::lock_guard<std::mutex> gurad1(objm1._mtx, std::adopt_lock);
    
    //此处为了让死锁必现，先睡一会
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::lock_guard<std::mutex> gurad2(objm2._mtx, std::adopt_lock);
    swap(objm1._obj, objm2._obj);

    std::cout << "thread [" << std::this_thread::get_id() << "] end" << std::endl;
}
void test_safe_swap()
{
    big_object_mgr objm1(5);
    big_object_mgr objm2(100);
    std::thread t1(safe_swap, std::ref(objm1), std::ref(objm2));
    std::thread t2(safe_swap, std::ref(objm2), std::ref(objm1));
    t1.join();
    t2.join();

    objm1.printinfo();
    objm2.printinfo();
}
//c++ 17 优化
void safe_swap_scope(big_object_mgr& objm1, big_object_mgr& objm2)
{
    std::cout << "thread [" << std::this_thread::get_id() << "] begin" << std::endl;
    if (&objm1 == &objm2)
    {
        return;
    }
    std::scoped_lock gurad(objm1._mtx, objm2._mtx);

    swap(objm1._obj, objm2._obj);
    std::cout << "thread [" << std::this_thread::get_id() << "] end" << std::endl;
}
void test_safe_swap_scope()
{
    big_object_mgr objm1(5);
    big_object_mgr objm2(100);
    std::thread t1(safe_swap_scope, std::ref(objm1), std::ref(objm2));
    std::thread t2(safe_swap_scope, std::ref(objm2), std::ref(objm1));
    t1.join();
    t2.join();

    objm1.printinfo();
    objm2.printinfo();
}

//层级锁
class hierachical_mutex
{
public:
    explicit hierachical_mutex(unsigned long value) :_hierachy_value(value),
        _previous_hierachy_value(0)
    {
    }

    hierachical_mutex(const hierachical_mutex&) = delete;
    hierachical_mutex& operator= (const hierachical_mutex&) = delete;

    void lock()
    {
        chech_for_hierachy_violation();
        _internal_mutex.lock();
        update_hierachy_value();
    }

    void unlock()
    {
        if (_this_thread_hierachy_value != _hierachy_value)
        {
            throw std::logic_error("mutex hierarchy violated");
        }

        _this_thread_hierachy_value = _previous_hierachy_value;
        _internal_mutex.unlock();
    }

    bool try_lock()
    {
        chech_for_hierachy_violation();
        if (!_internal_mutex.try_lock())
        {
            return false;
        }

        update_hierachy_value();
        return true;
    }

private:
    std::mutex _internal_mutex;
    //当前层级值
    unsigned long const _hierachy_value;
    //上一级层级值
    unsigned long _previous_hierachy_value;
    //本线程记录的层级值 不依赖于对象
    static thread_local unsigned long _this_thread_hierachy_value;

    void chech_for_hierachy_violation()
    {
        if (_this_thread_hierachy_value <= _hierachy_value)
        {
            throw std::logic_error("mutex hierachy violated");
        }
    }
    void update_hierachy_value()
    {
        _previous_hierachy_value = _this_thread_hierachy_value;
        _this_thread_hierachy_value = _hierachy_value;
    }
};

thread_local unsigned long hierachical_mutex::_this_thread_hierachy_value(ULLONG_MAX);

void test_hierarchy_lock()
{
    hierachical_mutex hmtx1(1000);
    hierachical_mutex hmtx2(500);

    std::thread t1([&hmtx1, &hmtx2]() {
        hmtx1.lock();
        hmtx2.lock();
        hmtx2.unlock();
        hmtx1.unlock();
        });

    std::thread t2([&hmtx1, &hmtx2]() {
        hmtx2.lock();
        hmtx1.lock();
        hmtx1.unlock();
        hmtx2.unlock();
        });
}

int main()
{
    //test_lock();

    return 0;
}


