#pragma once

#include <algorithm>
#include "manager_impl.h"

namespace test
{
    using namespace chess;

    class PerftTestData{
        public:
        typedef struct {
            uint32_t depth;
            uint64_t nodes;
            std::string fen;
        } PerftData;

        static const PerftData data[25];
    };

    class Perft
    {
        template <bool>
        uint64_t perft(int depth);

        bool m_print;
        Board *m_board;
    public:

        Perft(Board* board = nullptr);
        Perft &operator=(Perft &&other);

        uint64_t run(int depth = 6, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); 

        /**
         * @brief Set if the perft test should print the results
         */
        void setPrint(bool print) { m_print = print; }    
    };
}