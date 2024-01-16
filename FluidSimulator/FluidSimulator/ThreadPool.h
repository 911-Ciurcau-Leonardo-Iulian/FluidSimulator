#pragma once

#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <iostream>

class ThreadPool {
private:
    std::mutex m;
    std::condition_variable cond;
    std::queue<std::function<void()>> q;
    bool isDone = false;
    std::vector<std::thread> threads;
    int totalWaitingThreads = 0;
    std::mutex totalWaitingThreadsMutex;

    void run();
    void safeIncrementTotalWaitingThreads();
    void safeDecrementTotalWaitingThreads();
    int safeGetTotalWaitingThreads();
public:
    ThreadPool() {}
    ThreadPool(int tasks);
    ~ThreadPool();
    void enqueueFunction(std::function<void()> func);
    int size() { return threads.size(); }
    void waitUntilAllThreadsWait();
};
