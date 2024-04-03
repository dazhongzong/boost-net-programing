
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <memory>

int num = 1;
std::mutex mtx_num;

std::condition_variable cvA;
std::condition_variable cvB;

void ResonableImplemention() {
    std::thread t1([]() {
        for (;;) {

            std::unique_lock<std::mutex> lock(mtx_num);
            cvA.wait(lock, []() {
                return num == 1;
                });

            num++;
            std::cout << "thread A print 1....." << std::endl;
            cvB.notify_one();
        }

        });

    std::thread t2([]() {
        for (;;) {

            std::unique_lock<std::mutex> lock(mtx_num);
            cvB.wait(lock, []() {
                return num == 2;
                });

            num--;
            std::cout << "thread B print 2....." << std::endl;
            cvA.notify_one();
        }

        });

    t1.join();
    t2.join();
}



template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue()
    {
    }

    threadsafe_queue(const threadsafe_queue& rhs)
    {
        std::lock_guard<std::mutex> lock(mut);
        data_queue = rhs.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]() { return !this->data_queue.empty(); });
        
        auto ret = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return ret;
    }
    
    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
        {
            return false;
        }
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }


    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
        {
            return nullptr;
        }
        auto ret = std::make_shared<T>(data_queue.front());
        data_queue.pop();
        return ret;
    }

    std::shared_ptr<T> top()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]() {return !this->data_queue.empty(); });
        return std::make_shared<T>(data_queue.front());
    }

};

void test_safe_que()
{
    threadsafe_queue<int> safe_que;
    std::mutex mtx_print;
    std::thread producer([&]() {
        for (int i = 0;; i++)
        {
            safe_que.push(i);
            {
                std::lock_guard<std::mutex> lock(mtx_print);
                std::cout << "producer push data is " << i << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
        }
    );

    std::thread consume1([&]()
        {
            for (;;)
            {
                auto data = safe_que.wait_and_pop();
                {
                    std::lock_guard<std::mutex> lock(mtx_print);
                    std::cout << "consumer1 wait and pop data is " << *data << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    );

    std::thread consume2([&]()
        {
            for (;;)
            {
                auto data = safe_que.try_pop();
                if(data !=nullptr)
                {
                    std::lock_guard<std::mutex> lock(mtx_print);
                    std::cout << "consumer2 try pop data is " << *data << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    );

    producer.join();
    consume1.join();
    consume2.join();
}


int main()
{
   
   
}


