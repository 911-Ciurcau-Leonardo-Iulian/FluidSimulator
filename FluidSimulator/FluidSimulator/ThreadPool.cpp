#include "threadPool.h"

ThreadPool::ThreadPool(int tasks)
{
    for (int i = 0; i < tasks; i++)
    {
        threads.push_back(std::thread([this]() { run(); }));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(m);
        isDone = true;
        cond.notify_all();
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

void ThreadPool::run()
{
    while (true)
    {
        std::function<void()> toExecute;

        {
            std::unique_lock<std::mutex> lock(m);
            safeIncrementTotalWaitingThreads();
            cond.wait(lock, [this]() { return !q.empty() || isDone; });
            safeDecrementTotalWaitingThreads();
            if (q.empty())
            {
                break;
            }
            toExecute = std::move(q.front());
            q.pop();
        }

        toExecute();
    }
}

void ThreadPool::safeIncrementTotalWaitingThreads()
{
    totalWaitingThreadsMutex.lock();
    totalWaitingThreads++;
    totalWaitingThreadsMutex.unlock();
}

void ThreadPool::safeDecrementTotalWaitingThreads()
{
    totalWaitingThreadsMutex.lock();
    totalWaitingThreads--;
    totalWaitingThreadsMutex.unlock();
}

int ThreadPool::safeGetTotalWaitingThreads()
{
    int retVal;
    totalWaitingThreadsMutex.lock();
    retVal = totalWaitingThreads;
    totalWaitingThreadsMutex.unlock();
    return retVal;
}

void ThreadPool::enqueueFunction(std::function<void()> func)
{
    std::unique_lock<std::mutex> lock(m);
    q.push(std::move(func));
    cond.notify_one();
}

void ThreadPool::waitUntilAllThreadsWait()
{
    while (safeGetTotalWaitingThreads() != threads.size() && !q.empty());
}
