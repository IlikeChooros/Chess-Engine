#pragma once

#include <unordered_map>
#include <future>

#include "threads.h"
#include "log.h"
#include "eval.h"
#include "hash.h"
#include "cache.h"
#include "transp_table.h"
#include "move_ordering.h"
#include "search_options.h"
#include "interrupt.h"


namespace chess
{

    // Helper class to store results of the search, since 
    // atomic<Result> is not copyable
    template <class T = Result>
    class shared_data
    {
        T m_data;
        std::mutex m_mutex;
    public:
        // Constructors
        shared_data() = default;
        shared_data(const T& data): m_data(data) {}
        shared_data(shared_data&& other)
        {
            *this = std::move(other);
        }

        shared_data& operator=(shared_data&& other)
        {
            m_data = other.m_data;
            return *this;
        }

        // Set the data, thread safe
        shared_data& operator=(const T& data)
        {
            set(data);
            return *this;
        }

        // Set the data, thread safe
        void set(const T& data)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_data = data;
        }

        // Get stored data, thread safe
        T& get()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_data;
        }
    };


    // Search thread
    class Thread
    {
    public:
        Thread();
        ~Thread();

        void start_thinking(Board& board, SearchCache& search_cache, Limits& limits);
        void stop();
        void join();

        // Returns true if the thread is thinking
        bool is_thinking() { return m_thinking; }

        // Get the atomic Result object
        shared_data<Result>& get_result() { return m_best_result; }

    private:

        void iterative_deepening();
        Value qsearch(Board& board, Value alpha, Value beta, int depth);

        template <NodeType>
        Value search(Board& board, Value alpha, Value beta, int depth);

        MoveList get_pv(int max_depth = 10);

        Board m_board;
        SearchCache *m_search_cache;
        Limits m_limits;
        Interrupt m_interrupt;
        Result m_result;

        std::thread m_thread;
        std::atomic<bool> m_thinking;
        shared_data<Result> m_best_result;

        friend class Engine;
    };
}
