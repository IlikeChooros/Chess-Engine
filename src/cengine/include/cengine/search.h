#pragma once

#include <unordered_map>
#include <future>

#include "threads.h"
#include "log.h"
#include "eval.h"
#include "move_gen.h"
#include "hash.h"
#include "cache.h"
#include "transp_table.h"
#include "move_ordering.h"
#include "search_utils.h"


namespace chess
{
    // Search thread
    class Thread
    {
    public:
        Thread();
        ~Thread();

        void start_thinking(Board& board, SearchCache& search_cache, SearchLimits& limits);
        void stop();
        void join();

        // Returns true if the thread is thinking
        bool is_thinking() { return m_thinking; }

        // Get the future object, user may call get() to get the result
        std::future<Result> get_future() { return m_promise.get_future(); }

    private:

        void iterative_deepening();
        Value qsearch(Board board, Value alpha, Value beta, int depth);

        template <NodeType>
        Value search(Board board, Value alpha, Value beta, int depth);

        MoveList get_pv(int max_depth = 10);        

        Board m_board;
        SearchCache *m_search_cache;
        SearchLimits m_limits;
        Interrupt m_interrupt;
        Result m_result;

        std::promise<Result> m_promise;
        std::thread m_thread;
        std::atomic<bool> m_thinking;

        friend class Engine;
    };


    void search(Board* board, GameHistory* gh, SearchCache* sc, SearchLimits* params, SearchResult* sr);
}
