#include <cengine/threads.h>

TaskQueue::TaskQueue(size_t workers) {
    this->m_stop = false;
    m_tasks_left = 0;
    m_workers.reserve(workers);
    for(size_t i = 0; i < workers; i++){
        m_workers.emplace_back(&TaskQueue::worker, this);
    }
}

TaskQueue::~TaskQueue() {
    stop();
}

/**
 * @brief Put a task on the queue, will be executed asynchornously by a worker thread
 */
void TaskQueue::enqueue(taskType task)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.emplace(task);
        m_tasks_left++;
    }
    m_cv.notify_one();
}

/**
 * @brief Stop all worker threads
 */
void TaskQueue::stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        this->m_stop = true;
    }    
    m_cv.notify_all();
    for(std::thread& w : m_workers){
        if(w.joinable()){
            w.join();
        }
    }
}

/**
 * @brief Worker thread function, waits fot the tasks and executes them
 */
void TaskQueue::worker()
{
    while(true) {
        taskType task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]{ return m_stop || !m_tasks.empty(); }); // waits till m_stop == true or there are tasks
            if (m_stop && m_tasks.empty())
                return;
            
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        task();
        m_tasks_left--;
    }
}