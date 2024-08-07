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

        static const PerftData data[26];
    };

    class Perft
    {
        template <bool>
        uint64_t perft(int depth);

        std::vector<std::string> m_results;
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

        std::vector<std::string> getResults() { return m_results; }
    };


    class TestPerftStockfish
    {
        Board *m_board;
    public:
        TestPerftStockfish(Board* b) : m_board(b) {}
        void run(int depth = 6, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", std::string stockfish_result = "");
    };
}