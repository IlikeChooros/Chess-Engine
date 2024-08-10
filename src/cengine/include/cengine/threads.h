#pragma once

#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

/**
 * @brief A simple task queue implementation
 */
class TaskQueue
{
    public:
        typedef std::function<void()> taskType;

        TaskQueue(size_t workers = 1);
        ~TaskQueue();

        /**
         * @brief Returns the number of tasks left in the queue
         */
        int tasksLeft() { 
            return m_tasks_left.load();
        }

        void enqueue(taskType task);
        void stop();

    private:
        void worker();

        std::atomic<size_t> m_tasks_left;
        std::vector<std::thread> m_workers;
        std::queue<taskType> m_tasks;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_stop;
};

// A thread pool is a task queue with multiple workers = number of hardware threads
class ThreadPool: public TaskQueue
{
    public:
        ThreadPool(size_t workers = std::thread::hardware_concurrency()): TaskQueue(workers) {}
};