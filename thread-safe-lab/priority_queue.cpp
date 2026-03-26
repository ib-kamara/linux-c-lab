#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>

using namespace std;

template <typename T>
class ThreadSafePriorityQueue
{
private:
    priority_queue<T> pq;
    mutex mtx;

public:
    void push(T value)
    {
        lock_guard<mutex> lock(mtx);
        pq.push(value);
    }

    bool pop(T& value)
    {
        lock_guard<mutex> lock(mtx);

        if (pq.empty())
        {
            return false;
        }

        value = pq.top();
        pq.pop();
        return true;
    }

    bool empty()
    {
        lock_guard<mutex> lock(mtx);
        return pq.empty();
    }

    size_t size()
    {
        lock_guard<mutex> lock(mtx);
        return pq.size();
    }
};

void priorityQueueTest()
{
    ThreadSafePriorityQueue<int> pq;
    vector<thread> threads;
    const int NUM_THREADS = 4;

    auto pusher = [&pq](int id)
    {
        for (int i = 0; i < 5; ++i)
        {
            int priority = rand() % 100;
            pq.push(priority);
            cout << "Thread " << id << " pushed: " << priority << "\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    };

    auto popper = [&pq]()
    {
        for (int i = 0; i < 20; ++i)
        {
            int value;

            if (pq.pop(value))
            {
                cout << "Popped: " << value << "\n";
            }

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back(pusher, i);
    }

    threads.emplace_back(popper);

    for (auto& t : threads)
    {
        t.join();
    }
}

int main()
{
    priorityQueueTest();
    return 0;
}