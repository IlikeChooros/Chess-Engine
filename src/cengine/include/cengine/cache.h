#pragma once

#include <cstring>

#include "types.h"
#include "transp_table.h"

// Cached move generation information
namespace chess
{
    struct CacheMoveGen
    {
        uint64_t attacks_from[64] = {0};
        uint64_t attacks_to[64] = {0};

        // Attacks by enemy pieces
        uint64_t danger;

        // Attacks by friendly pieces
        uint64_t activity;

        // Pinned pieces
        uint64_t pinned;

        // Possible captures by friendly pieces
        uint64_t possible_captures;
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
        SearchCache(): tt(64) {}
        
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