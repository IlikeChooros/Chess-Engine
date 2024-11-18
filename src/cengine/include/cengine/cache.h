#pragma once

#include <cstring>

#include "types.h"
#include "transp_table.h"

// Cached move generation information
namespace chess
{
    struct CacheMoveGen
    {
        // Attacks by enemy pieces
        uint64_t danger;

        // Attacks by friendly pieces
        uint64_t activity;
    };

    class HistoryHeuristic
    {
        public:
            HistoryHeuristic() = default;

            /**
             * @brief Update the history heuristic table
             */
            inline void update(bool is_white, Move move, int depth)
            {
                history[is_white][move.getFrom()][move.getTo()] += depth * depth;
            }

            /**
             * @brief Get the history heuristic value for a move
             */
            inline int get(bool is_white, Move move)
            {
                return history[is_white][move.getFrom()][move.getTo()];
            }

            /**
             * @brief Clear the history heuristic table
             */
            inline void clear()
            {
                memset(history, 0, sizeof(history));
            }

        private:
            uint64_t history[2][64][64];
    };


    /**
     * @brief Cache for search information
     */
    class SearchCache
    {
    public:
        // Default hash size, in MB
        static constexpr size_t DEFAULT_HASH_SIZE = 16;

        SearchCache(): tt(DEFAULT_HASH_SIZE) {}
        
        /**
         * @brief Get the history heuristic object
         */
        inline HistoryHeuristic& getHH() { return hh; }

        /**
         * @brief Get the transposition table object
         */
        inline TTable<TEntry>& getTT() { return tt; }

    private:
        HistoryHeuristic hh;
        TTable<TEntry> tt;
    };
}