#pragma once

#include <unordered_map>
#include <mutex>


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
        // Basic UCI options
        int depth = 8; // In plies, that is max possible depth
        uint64_t nodes = UINT64_MAX;
        int mate = 0;
        int64_t movetime = INT64_MAX; // In milliseconds
        uint64_t wtime = 0;
        uint64_t btime = 0;
        uint64_t winc = 0;
        uint64_t binc = 0;
        bool infinite = false;
        bool ponder = false;

        // Asynchronous search
        bool stop = false;
        uint64_t nodes_searched = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
        std::mutex mutex = {};

        SearchParams() = default;

        SearchParams(const SearchParams& other)
        {
            *this = other;
        }

        SearchParams& operator=(const SearchParams& other)
        {
            depth = other.depth;
            movetime = other.movetime;
            nodes = other.nodes;
            movetime = other.movetime;
            infinite = other.infinite;
            ponder = other.ponder;
            stop = other.stop;
            return *this;
        }

        void stopSearch()
        {
            std::lock_guard<std::mutex> lock(mutex);
            stop = true;
        }

        bool shouldStop()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return stop;
        }

        void resetStop()
        {
            std::lock_guard<std::mutex> lock(mutex);
            stop = false;
        }
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

    SearchResult search(Board* board, GameHistory* gh, SearchCache* sc, SearchParams* params);
}
