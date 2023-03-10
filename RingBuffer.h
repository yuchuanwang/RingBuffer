// Ring buffer queue with lock-free supported
// Yuchuan Wang
// 
#pragma once

#include <atomic>
#include <iostream>

template<typename T>
class RingBufferQueue {
public:
    RingBufferQueue(uint32_t max_capacity = 100, bool lock_free = false)
    {
        capacity = max_capacity + 1;
        is_lock_free = lock_free;
        head = 0;
        tail = 0;
        data = nullptr;
        if (capacity > 0)
        {
            data = new T[capacity];
        }
    }

    // Copy constructor
    RingBufferQueue(const RingBufferQueue& q)
    {
        capacity = q.capacity;
        is_lock_free = q.is_lock_free;
        head = q.head.load();
        tail = q.tail.load();
        if (capacity > 0)
        {
            data = new T[q.capacity];
            std::copy(q.data, q.data + capacity, data);
        }
    }

    // Move constructor
    RingBufferQueue(RingBufferQueue&& q) noexcept
    {
        // Move pointer to destination
        capacity = q.capacity;
        is_lock_free = q.is_lock_free;
        head = q.head.load();
        tail = q.tail.load();
        data = q.data;

        // Detach pointer from source data to avoid double free
        q.data = nullptr;
        q.capacity = 0;
        q.head = 0;
        q.tail = 0;
    }

    // Assignment operator
    RingBufferQueue& operator=(const RingBufferQueue& q)
    {
        if (this != &q)
        {
            delete[] data;
            data = nullptr;

            capacity = q.capacity;
            is_lock_free = q.is_lock_free;
            head = q.head.load();
            tail = q.tail.load();
            data = new T[q.capacity];
            std::copy(q.data, q.data + capacity, data);
        }
        return *this;
    }

    // Move assignment operator
    RingBufferQueue& operator=(RingBufferQueue&& q) noexcept
    {
        if (this != &q)
        {
            // Release current memory
            delete[] data;

            // Move pointer to destination
            data = q.data;
            capacity = q.capacity;
            is_lock_free = q.is_lock_free;
            head = q.head;
            tail = q.tail;

            // Detach pointer from source data to avoid double free
            q.data = nullptr;
            q.capacity = 0;
            q.head = 0;
            q.tail = 0;
        }
        return *this;
    }

    ~RingBufferQueue()
    {
        if (data)
        {
            delete[] data;
            data = nullptr;
        }
    }

    bool IsFull() const
    {
        // There is am empty slot between head and tail
        uint32_t tail_cur = tail.load();
        uint32_t head_cur = head.load();
        return ((tail_cur + 1) % capacity == head_cur);
    }

    bool IsEmpty() const
    {
        uint32_t tail_cur = tail.load();
        uint32_t head_cur = head.load();
        return (head_cur == tail_cur);
    }

    uint32_t GetCapacity() const
    {
        // There is a hidden slot as seperator
        return (capacity - 1);
    }

    bool IsLockFree() const
    {
        return is_lock_free;
    }

    bool Enqueue(const T& val)
    {
        if (IsFull())
        {
            return false;
        }

        if(is_lock_free)
        {
            EnqueueLockFree(val);
        }
        else
        {
            EnqueueNoLockFree(val);
        }
        return true;
    }

    bool Dequeue(T& result)
    {
        if (IsEmpty())
        {
            return false;
        }

        if(is_lock_free)
        {
            DequeueLockFree(result);
        }
        else
        {
            DequeueNoLockFree(result);
        }
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, const RingBufferQueue& q)
    {
        if (q.IsEmpty())
        {
            os << "[]" << std::endl;
        }
        else
        {
            uint32_t next = q.head;
            os << "[";
            do
            {
                if (next != q.head)
                {
                    // Add comma
                    os << ", ";
                }
                os << q.data[next];
                next = (next + 1) % q.capacity;
            } while (next != q.tail);
            os << "]" << std::endl;
        }
        return os;
    }

private:
    void EnqueueNoLockFree(const T& val)
    {
        // Save data to tail
        data[tail] = val;
        // Move tail forward
        tail = (tail + 1) % capacity;
    }

    void EnqueueLockFree(const T& val)
    {
        // Save original position
        uint32_t tail_cur = tail.load();
        // CAS for tail
        while (tail.compare_exchange_weak(tail_cur, (tail_cur + 1) % capacity))
        {
            // Save data to tail with original position
            data[tail_cur] = val;
        }
    }

    void DequeueNoLockFree(T& result)
    {
        // Return the value
        result = data[head];
        // Move head forward
        head = (head + 1) % capacity;
    }

    void DequeueLockFree(T& result)
    {
        // Save original position
        uint32_t head_cur = head.load();
        // CAS for head
        while (head.compare_exchange_weak(head_cur, (head_cur + 1) % capacity))
        {
            // Return the value at original position
            result = data[head_cur];
        }
    }

private:
    // Using lock-free or not
    bool is_lock_free;
    // Reserve one more slot to sperate head and tail
    uint32_t capacity;
    // The index ready to be read
    std::atomic<uint32_t> head;
    // The index ready to be write
    std::atomic<uint32_t> tail;
    T* data;
};
