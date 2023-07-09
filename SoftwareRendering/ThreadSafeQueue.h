#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
struct ThreadSafeQueue {
    ThreadSafeQueue() {}

    void Push(T new_value) {
        std::lock_guard<std::mutex> lock(mutex);
        m_queue.push(new_value);
        m_cond.notify_one();
    }

    void WaitAndPop(T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        m_cond.wait(lock, [this] { 
            return !m_queue.empty();
        });
        value = m_queue.front();
        m_queue.pop();
    }


    bool TryPop(T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        if (m_queue.empty())
            return false;
        value = m_queue.front();
        m_queue.pop();
        return true;
    }

    
    bool Empty() const {
        return m_queue.empty();
    }

private:
    mutable std::mutex mutex;
    std::queue<T> m_queue;
    std::condition_variable m_cond;
};
