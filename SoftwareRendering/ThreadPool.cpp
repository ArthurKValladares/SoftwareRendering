#include "ThreadPool.h"
#include <memory>

void ThreadPool::WorkerThread() {
    while (!m_done) {
        std::function<void()> task;
        if (m_work_queue.TryPop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
}

ThreadPool::ThreadPool() : 
    m_joiner(m_threads),
    m_done(false) {
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
        for (unsigned i = 0; i < thread_count; ++i) {
            m_threads.push_back(
                std::thread(&ThreadPool::WorkerThread, this));
        }
    } catch (...) {
        m_done = true;
        throw;
    }
}

ThreadPool::~ThreadPool() {
    m_done = true;
}


void ThreadPool::Schedule(const std::function<void()> task) {
    // TODO: Get rid of `make_shared` hack when the compiler supports it
    auto p_task = std::make_shared<std::packaged_task<void()>>(std::bind(std::move(task)));
    std::future<void> res(p_task->get_future());
    m_work_queue.Push([p_task](){ (*p_task)(); });
    m_futures.push_back(std::move(res));
}


void ThreadPool::Wait() {
    for (std::future<void>& future : m_futures) {
        future.wait();
    }
    m_futures.clear();
}
