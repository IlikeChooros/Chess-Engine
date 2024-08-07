#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

#include "board.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"
#include "move_gen.h"
#include "history.h"
#include "eval.h"
#include "search.h"
#include "magic_bitboards.h"

namespace chess
{
    // Implements the Manager class, has simple to use API for the user
    class ManagerImpl
    {
    public:

        ManagerImpl(Board* board = nullptr);
        ManagerImpl(const ManagerImpl& other) = delete;
        ManagerImpl& operator=(ManagerImpl&& other);


        void reload();

        /**
         * @brief Returns the current game status
         * @return The current game status
         */
        inline GameStatus getStatus() { return get_status(board, &history, &move_list, &cache); }

        /**
         * @brief Initializes all engine components
         */
        inline void init() {
            init_board();
            init_eval();
            init_hashing();
            init_magics(false);
        }

        /**
         * @brief Generates all legal moves for the current board state
         * @return The number of moves generated
         */
        inline int generateMoves() { 
            n_moves = ::gen_legal_moves(&move_list, board, &cache); 
            return n_moves;
        }

        /**
         * @brief Searches for the best move in the current position, 
         * if history is not read/modifed by another thread, then thread safe
         * @return The best move found & its score
         */
        inline SearchResult search() noexcept {
            return ::chess::search(board, &history, &sc, search_params); 
        }

        /**
         * @brief Pushes the current move to the history stack, thread safe
         */
        inline void pushHistory() { 
            std::lock_guard<std::mutex> l(mutex);
            history.push(board, curr_move); 
        }

        /**
         * @brief Makes a move, updating the board, the side to move and the move history, thread safe
         * @attention User should call `generateMoves()` after calling this function
         */
        inline void make(Move& move) { 
            std::lock_guard<std::mutex> l(mutex); 
            ::make(move, board, &history); 
            curr_move = move;
        }

        /**
         * @brief Unmakes current move, restoring the board to the previous state, 
         * may be called only once after `make()`, thread safe
         * @attention User should call `generateMoves()` after calling this function
         */
        inline void unmake() noexcept { 
            std::lock_guard<std::mutex> l(mutex); 
            ::unmake(curr_move, board, &history); 
        }

        /**
         * @brief Sets the search parameters for the search function
         */
        inline void setSearchParams(SearchParams params) { search_params = params; }

        inline std::mutex& getMutex() { return mutex; }

        std::mutex mutex;
        CacheMoveGen cache;
        GameHistory history;
        MoveList move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        SearchParams search_params;
        SearchCache sc;
    };
}