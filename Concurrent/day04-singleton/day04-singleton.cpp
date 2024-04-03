
#include <mutex>
#include <iostream>

//局部静态变量
/*
我们知道当一个函数中定义一个局部静态变量，那么这个局部静态变量只会初始化一次，就是在这个函数第一次调用的时候，
以后无论调用几次这个函数，函数内的局部静态变量都不再初始化。 那我们可以利用局部静态变量这一特点实现单例

上述版本的单例模式在C++11 以前存在多线程不安全的情况，编译器可能会初始化多个静态变量。
但是C++11推出以后，各厂商优化编译器，能保证线程安全。所以为了保证运行安全请确保使用C++11以上的标准。
*/
class Single2 {
private:
    Single2()
    {
    }
    Single2(const Single2&) = delete;
    Single2& operator=(const Single2&) = delete;
public:
    static Single2& GetInst()
    {
        static Single2 single;
        return single;
    }
};

//饿汉式初始化
/*
在C++11 推出以前，局部静态变量的方式实现单例存在线程安全问题，所以部分人推出了一种方案，就是在主线程启动后，
其他线程没有启动前，由主线程先初始化单例资源，这样其他线程获取的资源就不涉及重复初始化的情况了。

饿汉式是从使用角度规避多线程的安全问题，很多情况下我们很难从规则角度限制开发人员，所以这种方式不是很推荐。
*/
//饿汉式
class Single2Hungry
{
private:
    Single2Hungry()
    {
    }
    Single2Hungry(const Single2Hungry&) = delete;
    Single2Hungry& operator=(const Single2Hungry&) = delete;
public:
    static Single2Hungry* GetInst()
    {
        if (single == nullptr)
        {
            single = new Single2Hungry();
        }
        return single;
    }
private:
    static Single2Hungry* single;
};
//饿汉式初始化
Single2Hungry* Single2Hungry::single = Single2Hungry::GetInst();
void thread_func_s2(int i)
{
    std::cout << "this is thread " << i << std::endl;
    std::cout << "inst is " << Single2Hungry::GetInst() << std::endl;
}
void test_single2hungry()
{
    std::cout << "s1 addr is " << Single2Hungry::GetInst() << std::endl;
    std::cout << "s2 addr is " << Single2Hungry::GetInst() << std::endl;
    for (int i = 0; i < 3; i++)
    {
        std::thread tid(thread_func_s2, i);
        tid.join();
    }
}


//懒汉式初始化
/*
很多人觉得什么时候调用初始化是用户的权利，不应该加以限制，所以就有了懒汉式方式初始化资源，
在用到时如果没有初始化单例则初始化，如果初始化了则直接使用. 所以这种方式我们要加锁，防止资源被重复初始化。

这种方式存在一个很严重的问题，就是当多个线程都调用单例函数时，我们不确定资源是被哪个线程初始化的。 
回收指针存在问题，存在多重释放或者不知道哪个指针释放的问题。
*/

class SinglePointer
{
private:
    SinglePointer()
    {
    }
    SinglePointer(const SinglePointer&) = delete;
    SinglePointer& operator=(const SinglePointer&) = delete;
public:
    static SinglePointer* GetInst()
    {
        if (single != nullptr)
        {
            return single;
        }
        s_mutex.lock();
        if (single != nullptr)
        {
            s_mutex.unlock();
            return single;
        }
        single = new SinglePointer();
        s_mutex.unlock();
        return single;
    }
private:
    static SinglePointer* single;
    static std::mutex s_mutex;
};

SinglePointer* SinglePointer::single = nullptr;
std::mutex SinglePointer::s_mutex;
void thread_func_lazy(int i)
{
    std::cout << "this is lazy thread " << i << std::endl;
    std::cout << "inst is " << SinglePointer::GetInst() << std::endl;
}
void test_singlelazy()
{
    for (int i = 0; i < 3; i++)
    {
        std::thread tid(thread_func_lazy, i);
        tid.join();
    }
    //何时释放new的对象？造成内存泄漏
    SinglePointer::GetInst()->~SinglePointer();
}


//智能指针
/*
我们能想到一个自动初始化资源并且自动释放的方式就是智能指针。利用智能指针自动回收资源。
*/

//可以利用智能指针完成自动回收

class SingleAuto
{
private:
    SingleAuto()
    {
    }
    SingleAuto(const SingleAuto&) = delete;
    SingleAuto& operator=(const SingleAuto&) = delete;
public:
    ~SingleAuto()
    {
        std::cout << "single auto delete success " << std::endl;
    }
    static std::shared_ptr<SingleAuto> GetInst()
    {
        if (single != nullptr)
        {
            return single;
        }
        s_mutex.lock();
        if (single != nullptr)
        {
            s_mutex.unlock();
            return single;
        }
        single = std::shared_ptr<SingleAuto>(new SingleAuto);
        s_mutex.unlock();
        return single;
    }
private:
    static std::shared_ptr<SingleAuto> single;
    static std::mutex s_mutex;
};
std::shared_ptr<SingleAuto> SingleAuto::single = nullptr;
std::mutex SingleAuto::s_mutex;
void test_singleauto()
{
    auto sp1 = SingleAuto::GetInst();
    auto sp2 = SingleAuto::GetInst();
    std::cout << "sp1  is  " << sp1 << std::endl;
    std::cout << "sp2  is  " << sp2 << std::endl;
    //此时存在隐患，可以手动删除裸指针，造成崩溃
    // delete sp1.get();
}


/*
这样开辟的资源交给智能指针管理免去了回收资源的麻烦。 
但是有些人觉得虽然智能指针能自动回收内存，如果有开发人员手动delete指针怎么办？ 
所以有人提出了利用辅助类帮助智能指针释放资源，将智能指针的析构设置为私有。

SafeDeletor就是删除的辅助类，实现了仿函数。构造智能指针时指定了SafeDeletor对象，这样就能帮助智能指针释放了。

但是上面的代码存在危险，比如懒汉式的使用方式，当多个线程调用单例时，有一个线程加锁进入3处的逻辑。
其他的线程有的在1处，判断指针非空则跳过初始化直接使用单例的内存会存在问题。 
主要原因在于SingleAutoSafe * temp = new SingleAutoSafe() 这个操作是由三部分组成的 
1 调用allocate开辟内存 2 调用construct执行SingleAutoSafe的构造函数 3 调用赋值操作将地址赋值给temp

而现实中2和3的步骤可能颠倒，所以有可能在一些编译器中通过优化是1，3，2的调用顺序， 
其他线程取到的指针就是非空，还没来的及调用构造函数就交给外部使用造成不可预知错误。 
为解决这个问题，C++11 推出了std::call_once函数保证多个线程只执行一次
*/
//为了规避用户手动释放内存，可以提供一个辅助类帮忙回收内存
//并将单例类的析构函数写为私有
class SingleAutoSafe;
class SafeDeletor
{
public:
    void operator()(SingleAutoSafe* sf)
    {
        std::cout << "this is safe deleter operator()" << std::endl;
        delete sf;
    }
};
class SingleAutoSafe
{
private:
    SingleAutoSafe() {}
    ~SingleAutoSafe()
    {
        std::cout << "this is single auto safe deletor" << std::endl;
    }
    SingleAutoSafe(const SingleAutoSafe&) = delete;
    SingleAutoSafe& operator=(const SingleAutoSafe&) = delete;
    //定义友元类，通过友元类调用该类析构函数
    friend class SafeDeletor;
public:
    static std::shared_ptr<SingleAutoSafe> GetInst()
    {
        //1处
        if (single != nullptr)
        {
            return single;
        }
        s_mutex.lock();
        //2处
        if (single != nullptr)
        {
            s_mutex.unlock();
            return single;
        }
        //额外指定删除器  
        //3 处
        single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, SafeDeletor());
        //也可以指定删除函数
        // single = std::shared_ptr<SingleAutoSafe>(new SingleAutoSafe, SafeDelFunc);
        s_mutex.unlock();
        return single;
    }
private:
    static std::shared_ptr<SingleAutoSafe> single;
    static std::mutex s_mutex;
};


/*
call_once
C++11 提出了call_once函数，我们可以配合一个局部的静态变量once_flag实现线程安全的初始化。 
多线程调用call_once函数时，会判断once_flag是否被初始化，如没被初始化则进入初始化流程，调用我们提供的初始化函数。 
但是同一时刻只有一个线程能进入这个初始化函数。
*/
class SingletonOnce {
private:
    SingletonOnce() = default;
    SingletonOnce(const SingletonOnce&) = delete;
    SingletonOnce& operator = (const SingletonOnce& st) = delete;
    static std::shared_ptr<SingletonOnce> _instance;

public:
    static std::shared_ptr<SingletonOnce> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _instance = std::shared_ptr<SingletonOnce>(new SingletonOnce);
            });

        return _instance;
    }

    void PrintAddress() {
        std::cout << _instance.get() << std::endl;
    }

    ~SingletonOnce() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

std::shared_ptr<SingletonOnce> SingletonOnce::_instance = nullptr;
void TestSingle() {

    std::thread t1([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SingletonOnce::GetInstance()->PrintAddress();
        });

    std::thread t2([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SingletonOnce::GetInstance()->PrintAddress();
        });

    t1.join();
    t2.join();
}


//为了让单例更加通用，可以做成模板类
template <typename T>
class Singleton {
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;
    static std::shared_ptr<T> _instance;
public:
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _instance = std::shared_ptr<T>(new T);
            });
        return _instance;
    }
    void PrintAddress() {
        std::cout << _instance.get() << std::endl;
    }
    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

/*
如果你只是实现一个简单的单例类推荐使用返回局部静态变量的方式 如果想大规模实现多个单例类可以用call_once实现的模板类。
*/

int main()
{
    

    return 0;
}

