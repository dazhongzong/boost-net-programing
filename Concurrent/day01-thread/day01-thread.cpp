#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
void thread_work1(std::string str)
{
    std::cout << "thread id:" << std::this_thread::get_id() <<
        " " << str << std::endl;
}

struct backgroud_task 
{
    void operator()()
    {
        std::cout << "thread id:" << std::this_thread::get_id() <<
            " " << "backgroud_task "<< std::endl;
    }
};

class func {
public:
    func(int& i):_i(i){}
    void operator()()
    {
        for (std::size_t i = 0; i < 3; i++)
        {
            std::cout << "func : i is " << _i++ << std::endl;
        }
    }
private:
    int& _i;
};

void oops()
{
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    //隐患 访问的是局部变量本身，当该{}销毁时，局部变量将非法 
    //debug 版本可能没问题 ，release版本可能有问题
    //可以改为传递局部变量的智能指针
    functhread.detach();
}

void catch_exception()
{
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    try
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (const std::exception&e)
    {
        functhread.join();
        throw;
    }
    functhread.join();
}
//利用RAII 技术完成 自动回收子线程资源
class thread_guard {
private:
    std::thread& _t;
public:
    explicit thread_guard(std::thread& t):_t(t){}
    ~thread_guard()
    {
        if (_t.joinable())
        {
            _t.join();
        }
    }

    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
};

void auto_gurad()
{
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g{ t };
    std::cout << "auto guard finished" << std::endl;

}

void print_str(int i, std::string s)
{
    std::cout << "i is " << i << " str is " << s << std::endl;
}

void danger_oops(int som_param)
{
    char buffer[1024] = {0};
    sprintf(buffer, "%i", som_param);
    //隐式转换，只有在该线程启动时才会发生转换 因为传入的是一个指针
    std::thread t(print_str, 3, buffer);
    t.detach();
    std::cout << "danger oops friend" << std::endl;
}

void safe_oops(int som_param)
{
    char buffer[1024] = { 0 };
    sprintf(buffer, "%i", som_param);
    std::thread t(print_str, 3, std::string(buffer));
    t.detach();
}

void change_param(int& param)
{
    param++;
}

void ref_oops(int some_param)
{
    std::cout << "before change,param is " << some_param << std::endl;
    //为什么要用std::ref,因为直接传入本身，底层会把他转为右值，左值引用不能绑定右值
    std::thread t(change_param, std::ref(some_param));
    t.join();

    std::cout << "after change,param is " << some_param << std::endl;
}

void some_function() 
{
    while (true) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void some_other_function() 
{
    while (true) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void dangerous_use()
{
    //t1 绑定some_function
    std::thread t1(some_function);
    //2 转移t1管理的线程给t2，转移后t1无效
    std::thread t2 = std::move(t1);
    //3 t1 可继续绑定其他线程,执行some_other_function
    t1 = std::thread(some_other_function);
    //4  创建一个线程变量t3
    std::thread t3;
    //5  转移t2管理的线程给t3
    t3 = std::move(t2);
    //6  转移t3管理的线程给t1   现在t1已经托管了一个线程
    t1 = std::move(t3);
    std::this_thread::sleep_for(std::chrono::seconds(2000));
}

std::thread f()
{
    return std::thread(some_function);
}

void param_function(int a)
{
    while (true)
    {
        std::cout << "param is " << a << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

class joining_thread
{
    std::thread _t;
public:
    joining_thread() noexcept = default;
    template<typename Callable, typename ...Args>
    explicit joining_thread(Callable&& func, Args&& ...args)
        :_t(std::forward<Callable>(func), std::forward<Args>(args)...)
    {

    }

    explicit joining_thread(std::thread t)noexcept :_t(std::move(t))
    {

    }

    joining_thread(joining_thread&& other)noexcept :_t(std::move(other._t)) {}

    joining_thread& operator=(joining_thread&& other)noexcept
    {
        if (_t.joinable())
        {
            _t.join();
        }

        _t = std::move(other._t);
        return *this;
    }

    joining_thread& operator=(std::thread t)noexcept
    {
        if (_t.joinable())
        {
            _t.join();
        }
        _t = std::move(t);
        return *this;
    }

    ~joining_thread() noexcept
    {
        if (_t.joinable())
        {
            _t.join();
        }
    }

    void swap(joining_thread other)noexcept
    {
        _t.swap(other._t);
    }

    std::thread::id get_id()const noexcept
    {
        return _t.get_id();
    }

    bool joinalbe()const noexcept
    {
        return _t.joinable();
    }

    void join()
    {
        _t.join();
    }

    void detach()
    {
        _t.detach();
    }

    std::thread& as_thread() noexcept
    {
        return _t;
    }

    const std::thread& as_thread()const noexcept
    {
        return _t;
    }
};

void use_jointhread()
{
    //1 根据线程构造函数构造joiningthread
    joining_thread j1([](int maxindex) {
        for (int i = 0; i < maxindex; i++)
        {
            std::cout << "in thread id " << std::this_thread::get_id() <<
                "cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10);

    //2 根据thread构造joiningthread
    joining_thread j2(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++)
        {
            std::cout << "in thread id " << std::this_thread::get_id() <<
                "cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10)
        );

    //3 根据thread构造joiningthread
    joining_thread j3(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++)
        {
            std::cout << "in thread id " << std::this_thread::get_id() <<
                "cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10)
    );
    //移动赋值
    j1 = std::move(j3);
}

void use_vector()
{
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < 10; i++)
    {
        threads.emplace_back(param_function,i);
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;    //⇽-- - ①
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads =
        (length + min_per_thread - 1) / min_per_thread;    //⇽-- - ②
    unsigned long const hardware_threads =
        std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);    //⇽-- - ③
    unsigned long const block_size = length / num_threads;    //⇽-- - ④
    std::vector<T> results(num_threads);
    std::vector<std::thread>  threads(num_threads - 1);   // ⇽-- - ⑤
    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);    //⇽-- - ⑥
        threads[i] = std::thread(//⇽-- - ⑦
            accumulate_block<Iterator, T>(),
            block_start, block_end, std::ref(results[i]));
        block_start = block_end;    //⇽-- - ⑧
    }
    accumulate_block<Iterator, T>()(
        block_start, last, results[num_threads - 1]);    //⇽-- - ⑨

    for (auto& entry : threads)
        entry.join();    //⇽-- - ⑩
    return std::accumulate(results.begin(), results.end(), init);    //⇽-- - ⑪
}

void use_parallel_acc() {
    std::vector <int> vec(10000);
    for (int i = 0; i < 10000; i++) {
        vec.push_back(i);
    }
    int sum = 0;
    sum = parallel_accumulate<std::vector<int>::iterator, int>(vec.begin(),
        vec.end(), sum);

    std::cout << "sum is " << sum << std::endl;
}

void day01()
{
    std::string hellostr = "hello world";
    //1 通过 函数 初始化并启动一个线程
    std::thread t1(thread_work1, hellostr);
    //主线程等待子线程完成并回收线程资源
    t1.join();
    //2 通过 函数对象 初始化一个线程
    /* 错误初始化方式 ，编译器会把他解释成 定义一个函数
    std::thread t2(backgroud_task());
    t2.join();
    */

    std::thread t2((backgroud_task()));
    t2.join();

    //3 利用 c++11 提供的初始化方式
    std::thread t3{ backgroud_task() };
    t3.join();

    //4 通过 lambda表达式初始化
    std::thread t4([](std::string str) {
        std::cout << "lambda" << " " + str << std::endl;
        }, hellostr);
    t4.join();

    //5 detach 注意事项
    oops();
    //防止主线程退出过快
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //6 捕获异常
    catch_exception();

    //7
    ref_oops(1);
}

void day02()
{
    //8
    dangerous_use();
    //9
    use_jointhread();
    //10 并行计算
    use_parallel_acc();
}

int main()
{
    return 0;
}


