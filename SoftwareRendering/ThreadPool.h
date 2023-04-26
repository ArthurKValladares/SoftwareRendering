#pragma once

#include <functional>
#include <mutex>
#include <vector>
#include <queue>

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void Schedule(const std::function<void()>&);
    void Wait() const;

private:
    std::vector<std::thread>            m_workers;
    std::queue<std::function<void()>>   m_taskQueue;
    std::atomic_uint                    m_taskCount;
    std::mutex                          m_mutex;
    std::condition_variable             m_condition;
    std::atomic_bool                    m_stop;
};