#pragma once


#include "search.h"
#include "board.h"
#include "move.h"
#include "perft.h"


namespace chess
{
    class Engine
    {
    public:

        Engine();

        /**
         * @brief Run a perft test at the specified depth
         * @return Total number of nodes
         */
        uint64_t perft(int depth);

        /**
         * @brief Start the search for the best move, position should be already set
         * @return The best move found
         */
        Move go(SearchOptions& options);

        /**
         * @brief Set the position of the board
         */
        void setPosition(const std::string& fen = Board::startFen);

    private:
        Board m_board;
    };
}