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

        Engine() = default;
        Engine& operator=(Engine&& other);
        Engine(const Engine&) = delete;
        ~Engine();

        static void base_init();
        void reset();
        uint64_t perft(int depth, bool print = true);
        shared_data<Result>& go(const SearchOptions& options);
        void join();
        void stop();
        bool setPosition(const std::string& fen = Board::START_FEN);
        bool setPosition(std::istringstream& fen);
        void setPosition(const Board& board);

        // uci options

        void setHashSize(size_t hash);
        void setLogFile(const std::string& file);

        /**
         * @brief Get the board
         */
        Board& board() { return m_board; }

        Thread m_main_thread;
        Board m_board;
        SearchCache m_search_cache;
    };
}