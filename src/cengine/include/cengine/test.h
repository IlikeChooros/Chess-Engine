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

        static const PerftData data[23];
    };

    class Perft
    {
        uint64_t perft(int depth);
        void printResults(int depth, uint64_t nodes, const uint64_t* nodes_path, ManagerImpl &manager);

        bool m_print;
        uint64_t m_time_us;
        uint64_t m_expected;
        Board *m_board;
    public:
        static constexpr uint64_t nodes_perft[6] = {20, 400, 8902, 197281, 4865609, 119060324};
        static constexpr int perft_max_depth = 6;

        Perft(Board* board = nullptr);
        Perft &operator=(Perft &&other);

        uint64_t run(int depth = 6, std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        /**
         * @brief Set the expected number of nodes for the perft test
         */
        void setExpected(uint64_t nodes) { m_expected = nodes; }

        /**
         * @brief Get the time in milliseconds it took to run the perft test
         */
        uint64_t getTime() const { return m_time_us / 1000; }    

        /**
         * @brief Set if the perft test should print the results
         */
        void setPrint(bool print) { m_print = print; }    
    };
}