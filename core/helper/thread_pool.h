//
//  thread_pool.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-16.
//

#ifndef thread_pool_h
#define thread_pool_h

#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    void Enqueue(std::function<void()> task);
    ~ThreadPool();
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
    void Worker();
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; i++) {
        workers.emplace_back([this] { Worker(); });
    }
}

void ThreadPool::Enqueue(std::function<void()> task) {
    
    std::unique_lock<std::mutex> lock(queueMutex);
    tasks.push(std::move(task));
    condition.notify_one();
}

void ThreadPool::Worker() {
    
    while (true) {
        std::function<void()> task;
        std::unique_lock<std::mutex> lock(queueMutex);
        condition.wait(lock, [this] { return stop || !tasks.empty(); });
        
        if (stop && tasks.empty()) return;
        
        task = std::move(tasks.front());
        tasks.pop();
        task();
    }
}

ThreadPool::~ThreadPool() {
    std::unique_lock<std::mutex> lock(queueMutex);
    stop = true;
    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}


#endif /* thread_pool_h */
