#pragma once

#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

/**
 * @brief A simple task queue implementation
 */
class TaskQueue
{
    public:
        typedef std::function<void()> taskType;

        TaskQueue(size_t workers = 1);
        ~TaskQueue();

        void enqueue(taskType task);
        void stop();

    private:
        void worker();

        std::vector<std::thread> m_workers;
        std::queue<taskType> m_tasks;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        bool m_stop;
};