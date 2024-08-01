#pragma once

#include "manager_impl.h"

namespace test
{
    using namespace chess;

    class Perft
    {
        uint64_t perft(int depth);
        void printResults(int depth, uint64_t nodes, const uint64_t* nodes_path, ManagerImpl &manager);

        bool m_print;
        bool m_init_pos;
        uint64_t m_time_us;
        Board *m_board;
    public:
        static constexpr uint64_t nodes_perft[6] = {20, 400, 8902, 197281, 4865609, 119060324};
        static constexpr int perft_max_depth = 6;

        Perft(Board* board = nullptr);
        Perft &operator=(Perft &&other);

        uint64_t run(int depth = 5);
        
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