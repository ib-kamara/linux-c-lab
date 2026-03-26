#include <iostream>
#include <queue>
#include <stack>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>

using namespace std;

// Thread-safe Queue class
template <typename T>
class ThreadSafeQueue
{
private:
    queue<T> queueData;
    mutex mtx;

public:
    ThreadSafeQueue()
    {
    }

    void push(T value)
    {
        lock_guard<mutex> lock(mtx);
        queueData.push(value);
    }

    bool pop(T& value)
    {
        lock_guard<mutex> lock(mtx);

        if (queueData.empty())
        {
            return false;
        }

        value = queueData.front();
        queueData.pop();
        return true;
    }

    bool empty()
    {
        lock_guard<mutex> lock(mtx);
        return queueData.empty();
    }

    size_t size()
    {
        lock_guard<mutex> lock(mtx);
        return queueData.size();
    }
};

// Thread-safe Stack class
template <typename T>
class ThreadSafeStack
{
private:
    stack<T> stackData;
    mutex mtx;

public:
    ThreadSafeStack()
    {
    }

    void push(T value)
    {
        lock_guard<mutex> lock(mtx);
        stackData.push(value);
    }

    bool pop(T& value)
    {
        lock_guard<mutex> lock(mtx);

        if (stackData.empty())
        {
            return false;
        }

        value = stackData.top();
        stackData.pop();
        return true;
    }

    bool empty()
    {
        lock_guard<mutex> lock(mtx);
        return stackData.empty();
    }

    size_t size()
    {
        lock_guard<mutex> lock(mtx);
        return stackData.size();
    }
};

// Problem 1: Producer-Consumer Simulation
void producerConsumerProblem()
{
    ThreadSafeQueue<string> messageQueue;
    const int NUM_PRODUCERS = 3;
    const int NUM_CONSUMERS = 2;
    const int MESSAGES_PER_PRODUCER = 5;

    atomic<int> messages_produced(0);
    vector<thread> producers;
    vector<thread> consumers;

    auto producer = [&messageQueue, &messages_produced](int id)
    {
        for (int i = 0; i < MESSAGES_PER_PRODUCER; ++i)
        {
            string message = "Producer " + to_string(id) + " Message " + to_string(i);
            messageQueue.push(message);
            messages_produced++;
            cout << "Produced: " << message << endl;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    };

    auto consumer = [&messageQueue, &messages_produced](int id)
    {
        int consumed = 0;

        while (consumed < NUM_PRODUCERS * MESSAGES_PER_PRODUCER)
        {
            string message;

            if (messageQueue.pop(message))
            {
                cout << "Consumer " << id << " processed: " << message << endl;
                consumed++;
            }
            else if (messages_produced == NUM_PRODUCERS * MESSAGES_PER_PRODUCER)
            {
                break;
            }

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    };

    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        producers.emplace_back(producer, i);
    }

    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        consumers.emplace_back(consumer, i);
    }

    for (auto& t : producers)
    {
        t.join();
    }

    for (auto& t : consumers)
    {
        t.join();
    }
}

// Problem 2: Undo-Redo System
void undoRedoProblem()
{
    ThreadSafeStack<string> undoStack;
    ThreadSafeStack<string> redoStack;
    string currentText = "";

    auto editText = [&undoStack, &redoStack, &currentText](const string& newText)
    {
        undoStack.push(currentText);

        while (!redoStack.empty())
        {
            string temp;
            redoStack.pop(temp);
        }

        currentText = newText;
        cout << "Text updated to: " << currentText << endl;
    };

    auto undo = [&undoStack, &redoStack, &currentText]()
    {
        string prevText;

        if (undoStack.pop(prevText))
        {
            redoStack.push(currentText);
            currentText = prevText;
            cout << "Undo - Current text: " << currentText << endl;
        }
    };

    auto redo = [&undoStack, &redoStack, &currentText]()
    {
        string nextText;

        if (redoStack.pop(nextText))
        {
            undoStack.push(currentText);
            currentText = nextText;
            cout << "Redo - Current text: " << currentText << endl;
        }
    };

    editText("Hello");
    editText("Hello World");
    editText("Hello Universe");
    undo();
    undo();
    redo();
    editText("Hello Galaxy");
}

int main()
{
    cout << "Problem 1: Producer-Consumer Simulation\n";
    producerConsumerProblem();

    cout << "\nProblem 2: Undo-Redo System\n";
    undoRedoProblem();

    return 0;
}