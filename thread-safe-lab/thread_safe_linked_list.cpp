#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

template <typename T>
class ThreadSafeLinkedList
{
private:
    struct Node
    {
        T data;
        Node* next;

        Node(T value)
        {
            data = value;
            next = nullptr;
        }
    };

    Node* head = nullptr;
    mutex mtx;

public:
    ~ThreadSafeLinkedList()
    {
        while (!empty())
        {
            T value;
            pop_front(value);
        }
    }

    void push_front(T value)
    {
        lock_guard<mutex> lock(mtx);

        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }

    bool pop_front(T& value)
    {
        lock_guard<mutex> lock(mtx);

        if (!head)
        {
            return false;
        }

        Node* temp = head;
        value = temp->data;
        head = head->next;
        delete temp;
        return true;
    }

    bool empty()
    {
        lock_guard<mutex> lock(mtx);
        return head == nullptr;
    }

    size_t size()
    {
        lock_guard<mutex> lock(mtx);

        size_t count = 0;
        Node* current = head;

        while (current)
        {
            count++;
            current = current->next;
        }

        return count;
    }
};

void linkedListTest()
{
    ThreadSafeLinkedList<int> list;
    vector<thread> threads;

    auto pusher = [&list](int id)
    {
        for (int i = 0; i < 5; ++i)
        {
            list.push_front(id * 100 + i);
            cout << "Thread " << id << " pushed: " << (id * 100 + i) << "\n";
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    };

    auto popper = [&list]()
    {
        for (int i = 0; i < 10; ++i)
        {
            int value;

            if (list.pop_front(value))
            {
                cout << "Popped: " << value << "\n";
            }

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    };

    threads.emplace_back(pusher, 1);
    threads.emplace_back(pusher, 2);
    threads.emplace_back(popper);

    for (auto& t : threads)
    {
        t.join();
    }
}

int main()
{
    linkedListTest();
    return 0;
}