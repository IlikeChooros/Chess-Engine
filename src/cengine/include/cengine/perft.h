#pragma once

#include "board.h"

// Benchmarking namespace mainly for perft test
namespace bench
{
    // Perft test
    class Perft
    {
        template <bool>
        uint64_t perft(int depth);

        bool m_print;
        chess::Board m_board;
    public:

        Perft(bool print = true);
        Perft &operator=(Perft &&other);

        uint64_t run(int depth = 6, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); 

        /**
         * @brief Set if the perft test should print the results
         */
        void setPrint(bool print) { m_print = print; }
    };
}