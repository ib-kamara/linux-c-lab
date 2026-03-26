#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>

using namespace std;

#define BUFFER_SIZE 5

class ThreadSafeCircularBuffer
{
private:
    int buffer[BUFFER_SIZE];
    int in = 0;
    int out = 0;
    int count = 0;
    mutex mtx;
    condition_variable not_full;
    condition_variable not_empty;

public:
    void push(int value)
    {
        unique_lock<mutex> lock(mtx);

        not_full.wait(lock, [this]
        {
            return count < BUFFER_SIZE;
        });

        buffer[in] = value;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        lock.unlock();
        not_empty.notify_one();
    }

    bool pop(int& value)
    {
        unique_lock<mutex> lock(mtx);

        if (count == 0)
        {
            return false;
        }

        value = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        lock.unlock();
        not_full.notify_one();
        return true;
    }

    bool empty()
    {
        lock_guard<mutex> lock(mtx);
        return count == 0;
    }

    bool full()
    {
        lock_guard<mutex> lock(mtx);
        return count == BUFFER_SIZE;
    }
};

void circularBufferTest()
{
    ThreadSafeCircularBuffer cb;
    vector<thread> producers;
    vector<thread> consumers;
    const int NUM_ITEMS = 10;

    auto producer = [&cb]()
    {
        for (int i = 0; i < NUM_ITEMS; ++i)
        {
            int value = rand() % 100;
            cb.push(value);
            cout << "Produced: " << value << "\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    };

    auto consumer = [&cb]()
    {
        for (int i = 0; i < NUM_ITEMS; ++i)
        {
            int value;

            if (cb.pop(value))
            {
                cout << "Consumed: " << value << "\n";
            }

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    };

    producers.emplace_back(producer);
    consumers.emplace_back(consumer);

    for (auto& t : producers)
    {
        t.join();
    }

    for (auto& t : consumers)
    {
        t.join();
    }
}

int main()
{
    circularBufferTest();
    return 0;
}