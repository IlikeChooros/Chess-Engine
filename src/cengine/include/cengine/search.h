#pragma once

#include "unordered_map"

#include "eval.h"
#include "move_gen.h"
#include "hash.h"
#include "cache.h"
#include "transp_table.h"
#include "move_ordering.h"


namespace chess
{
    struct SearchParams
    {
        int depth; // In plies
        uint64_t time; // In milliseconds
        int nodes; 
        int movetime;
        bool infinite;
        bool ponder;
    };

    // Search result
    // move: Best move found
    // score: Score of the position
    // depth: Depth of the search
    // time: Time taken to search (in milliseconds)
    // status: Game status (ongoing, checkmate, stalemate, draw)
    struct SearchResult
    {
        Move move;
        int score;
        int depth;
        uint64_t time;
        GameStatus status = ONGOING;
    };

    constexpr SearchParams DEFAULT_SEARCH_PARAMS = { 5, 2000, 0, 0, false, false };

    SearchResult search(Board* board, GameHistory* gh, SearchCache* sc, SearchParams params = DEFAULT_SEARCH_PARAMS);
}
