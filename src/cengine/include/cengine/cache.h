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
        HistoryHeuristic() { clear(); }

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

    class KillerHeuristic
    {
    public:
        constexpr static int MAX_PLY = 64;
        constexpr static int MOVES_PER_PLY = 3;

        KillerHeuristic()
        {
            moves.reserve(MAX_PLY);
            for (int i = 0; i < MAX_PLY; i++)
                moves.push_back({0, 0, 0});
        }

        // Update the killer list, should be done only if:
        // - the `killer` move is a quiet one
        // - the move caused a beta-cutoff
        inline void update(Move killer, Depth ply)
        {
            static int killer_index = 0;
            
            // If empty or already in the list of killers, then exit
            for(int i = 0; i < MOVES_PER_PLY; ++i)
                if (moves[ply][i] == killer)
                    return;

            // If the killer not on the list, then override an element
            moves[ply][killer_index] = killer;
            killer_index             = (killer_index + 1) % MOVES_PER_PLY;
        }

        // Check if that move is da freaky killer
        inline bool is_killer(Move killer, Depth ply)
        {
            for(int i = 0; i < MOVES_PER_PLY; ++i)
                if (moves[ply][i] == killer)
                    return true;
            
            return false;
        }

        /**
         * @brief Clear the killer moves
         */
        inline void clear()
        {
            for (auto& v: moves)
                for (auto&n : v)
                    n = Move::nullMove;
        }

    private:
        std::vector<std::vector<Move>> moves;
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

        /**
         * @brief Get the killer table
         */
        inline KillerHeuristic& getKH() { return kh; }

    private:
        KillerHeuristic kh;
        HistoryHeuristic hh;
        TTable<TEntry> tt;
    };
}