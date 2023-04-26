#include "ThreadPool.h"

ThreadPool::ThreadPool() : 
    m_workers(),
    m_taskQueue(),
    m_taskCount(0u),
    m_mutex(),
    m_condition(),
    m_stop(false) 
{
    const std::size_t num_threads = std::thread::hardware_concurrency();
    m_workers.resize(num_threads);
    for (std::size_t i = 0; i < num_threads; i++) {
        m_workers.emplace_back([this]() -> void {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_condition.wait(lock, [this]() -> bool {
                        return ! m_taskQueue.empty() || m_stop;
                    });

                    if (m_stop && m_taskQueue.empty()) {
                        return;
                    }

                    task = std::move(m_taskQueue.front());
                    m_taskQueue.pop();
                }
                task();
                m_taskCount--;
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    m_stop = true;
    m_condition.notify_all();
    for (std::thread& w : m_workers) {
        w.join();
    }
}


void ThreadPool::Schedule(const std::function<void()>& task) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_taskQueue.push(task);
    }
    m_taskCount++;
    m_condition.notify_one();
}

void ThreadPool::Wait() const {
    do {
        std::this_thread::yield();
    } while (m_taskCount != 0u);
}