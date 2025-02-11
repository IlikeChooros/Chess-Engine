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

    // Used for 'improving'
    struct SearchStackEntry
    {
        static constexpr Value noValue = (1 << 17);
        Value static_eval;
    };


    class SearchStack
    {
    public:
        static constexpr auto STACK_SIZE = KillerHeuristic::MAX_PLY + 4;

        SearchStack()
        {
            ss.reset(new SearchStackEntry[STACK_SIZE]);
            _begin = ss.get();
            _end   = ss.get() + STACK_SIZE;
            _it    = _begin + 4; // so that (_it - 4) is possible
        }

        // Get the derefrenced value of the iterator
        SearchStackEntry& get(Depth ply)
        {
            return *(_it + ply);
        }

        // Get the `SearchStackEntry` pointer from 2 or 4 plies ago, if has it's value set
        // May return null pointer if the stack has no set values (2 and 4 plies ago)
        SearchStackEntry* get_improving_it(Depth ply)
        {
            SearchStackEntry* imprv = nullptr;

            if ((_it + ply - 2)->static_eval != SearchStackEntry::noValue)
            {
                imprv = (_it + ply - 2);
            }
            else if ((_it + ply - 4)->static_eval != SearchStackEntry::noValue)
            {
                imprv = (_it + ply - 4);
            }

            return imprv;
        }

        // Check if the position is improving (better than the one from 2 or 4 plies ago)
        bool improving(Board& board, Depth ply)
        {
            if (board.m_in_check)
                return false;
            
            auto imprv = get_improving_it(ply);
            return imprv != nullptr ? _it->static_eval > imprv->static_eval : true;
        }
        
        // Set all values of the 'stack' to `SearchStackEntry::noValue`
        void clear()
        {
            for(auto it = _begin; it != _end; it++)
                it->static_eval = SearchStackEntry::noValue;
        }

    private:
        std::unique_ptr<SearchStackEntry[]> ss;

        SearchStackEntry* _it;
        SearchStackEntry* _begin;
        SearchStackEntry* _end;
    };
}