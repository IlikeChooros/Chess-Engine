#pragma once

#include <future>

#include "search.h"
#include "board.h"
#include "move.h"
#include "perft.h"


namespace chess
{
    /*
    
    ### Engine

    Main class for interacting with the engine, setting the position and running the search
    
    
    */
    class Engine
    {
    public:

        Engine();
        Engine& operator=(Engine&& other);
        ~Engine();

        static void init();
        void reset();
        uint64_t perft(int depth, bool print = true);
        std::future<Result> go(SearchOptions& options);
        void join();
        void stop();
        void setPosition(const std::string& fen = Board::START_FEN);
        void setPosition(const Board& board);

        /**
         * @brief Get the board
         */
        Board& board() { return m_board; }

    private:
        Board m_board;
        Thread m_main_thread;
        SearchCache m_search_cache;
    };
}