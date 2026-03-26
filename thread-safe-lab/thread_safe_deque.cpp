#include <iostream>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

template <typename T>
class ThreadSafeDeque
{
private:
    deque<T> dequeData;
    mutex mtx;

public:
    void push_front(T value)
    {
        lock_guard<mutex> lock(mtx);
        dequeData.push_front(value);
    }

    void push_back(T value)
    {
        lock_guard<mutex> lock(mtx);
        dequeData.push_back(value);
    }

    bool pop_front(T& value)
    {
        lock_guard<mutex> lock(mtx);

        if (dequeData.empty())
        {
            return false;
        }

        value = dequeData.front();
        dequeData.pop_front();
        return true;
    }

    bool pop_back(T& value)
    {
        lock_guard<mutex> lock(mtx);

        if (dequeData.empty())
        {
            return false;
        }

        value = dequeData.back();
        dequeData.pop_back();
        return true;
    }

    bool empty()
    {
        lock_guard<mutex> lock(mtx);
        return dequeData.empty();
    }

    size_t size()
    {
        lock_guard<mutex> lock(mtx);
        return dequeData.size();
    }
};

void dequeTest()
{
    ThreadSafeDeque<int> dq;
    vector<thread> threads;

    auto push_front = [&dq](int id)
    {
        for (int i = 0; i < 5; ++i)
        {
            dq.push_front(id * 100 + i);
            cout << "Thread " << id << " pushed front: " << (id * 100 + i) << "\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    };

    auto push_back = [&dq](int id)
    {
        for (int i = 0; i < 5; ++i)
        {
            dq.push_back(id * 100 + i);
            cout << "Thread " << id << " pushed back: " << (id * 100 + i) << "\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    };

    auto pop_front = [&dq]()
    {
        for (int i = 0; i < 10; ++i)
        {
            int value;

            if (dq.pop_front(value))
            {
                cout << "Popped front: " << value << "\n";
            }

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    };

    auto pop_back = [&dq]()
    {
        for (int i = 0; i < 10; ++i)
        {
            int value;

            if (dq.pop_back(value))
            {
                cout << "Popped back: " << value << "\n";
            }

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    };

    threads.emplace_back(push_front, 1);
    threads.emplace_back(push_back, 2);
    threads.emplace_back(pop_front);
    threads.emplace_back(pop_back);

    for (auto& t : threads)
    {
        t.join();
    }
}

int main()
{
    dequeTest();
    return 0;
}