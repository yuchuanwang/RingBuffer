// 
#include <thread>
#include "RingBuffer.h"

void TestRingBufferQueue()
{
    RingBufferQueue<int> q;
    std::cout << "Queue capacity: " << q.GetCapacity() << std::endl;
    std::cout << "Add data into queue" << std::endl;
    for (int i = 0; !q.IsFull(); i++)
    {
        std::cout << i << " ";
        q.Enqueue(i);
    }
    std::cout << std::endl;

    std::cout << q;

    std::cout << "Pop these items from queue, and add it to tail: " << std::endl;
    for (int i = 0; i < 5; i++)
    {
        int val = 0;
        q.Dequeue(val);
        std::cout << val << " ";
        q.Enqueue(val);
    }
    std::cout << std::endl;
    std::cout << q;

    std::cout << "Pop all data from queue" << std::endl;
    for (int i = 0; !q.IsEmpty(); i++)
    {
        int val = 0;
        q.Dequeue(val);
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

void thread_push(RingBufferQueue<int>* q)
{
    for (int i = 0; i < 100; i++)
    {
        q->Enqueue(i*2);
        int val = 0;
        //q->Dequeue(val);
    }
}

void thread_pop(RingBufferQueue<int>* q)
{
    for (int i = 0; i < 1000; i++)
    {
        int val = 0;
        q->Dequeue(val);
    }
}

void TestRingBufferQueueThreading()
{
    RingBufferQueue<int> q(100, true);
    std::thread thdA(thread_push, &q);
    std::thread thdB(thread_push, &q);
    thdA.join();
    thdB.join();
    std::cout << q;
}

void TestingCopyAssignmentFunc()
{
    RingBufferQueue<int> q1(10);
    for(int i = 0; i < 10; i++)
    {
        q1.Enqueue(i * 2);
    }
    std::cout << q1;
    for(int i = 0; i < 5; i++)
    {
        int val = 0;
        q1.Dequeue(val);
        q1.Enqueue(i * 3);
    }
    std::cout << q1;

    // Copy constructor
    RingBufferQueue<int> q2(q1);
    std::cout << q2;
    // Move constructor
    RingBufferQueue<int> q3(std::move(q2));
    std::cout << q3;
    // Assignment
    RingBufferQueue<int> q4(20);
    q4 = q3;
    std::cout << q4;
}

int main()
{
    TestingCopyAssignmentFunc();

    TestRingBufferQueue();
    TestRingBufferQueueThreading();
    return 0;
}
