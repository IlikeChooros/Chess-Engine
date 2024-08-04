#pragma once

#include "unordered_map"

#include "eval.h"
#include "move_gen.h"
#include "hash.h"


namespace chess
{
    struct SearchParams
    {
        int depth;
        int time;
        int nodes;
        int movetime;
        bool infinite;
        bool ponder;
    };

    struct SearchResult
    {
        Move move;
        int score;
    };

    // Transposition table entry
    // hash: Zobrist hash of the position
    // depth: Depth of the search
    // nodeType: Type of node (PV, CUT, ALL)
    // score: Score of the position
    // bestMove: Best move found
    // age: Age of the entry
    struct TranspositionEntry
    {
        static constexpr int EXACT = 0;
        static constexpr int LOWERBOUND = 1;
        static constexpr int UPPERBOUND = 2;

        uint64_t hash;
        int depth;
        int nodeType;
        int score;
        Move bestMove;
        int age;
    };

    constexpr SearchParams DEFAULT_SEARCH_PARAMS = { 3, 0, 0, 0, false, false };

    SearchResult search(Board* board, GameHistory* gh, SearchParams params = DEFAULT_SEARCH_PARAMS);
}
