#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <chrono>

#include "board.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"
#include "move_gen.h"
#include "history.h"
#include "eval.h"
#include "search.h"

namespace chess
{
    // Implements the Manager class, has simple to use API for the user
    class ManagerImpl
    {
    public:

        typedef enum{
            Normal = 0,
            Checkmate = 1,
            Draw = 2,
            Repetition = 3,
        } GameState;

        ManagerImpl(Board* board = nullptr);
        ManagerImpl(const ManagerImpl& other) = delete;
        ManagerImpl& operator=(ManagerImpl&& other);


        void reload();
        GameState evalState();

        /**
         * @brief Initializes the board bitboards, should be called once before generating moves
         */
        inline void init() {
            init_board(board);
            init_eval();
            init_hashing();
        }

        /**
         * @brief Generates all legal moves for the current board state
         * @return The number of moves generated
         */
        inline int generateMoves() { 
            n_moves = ::gen_legal_moves(&move_list, board); 
            return n_moves;
        }

        /**
         * @brief Searches for the best move in the current position
         * @return The best move found
         */
        inline SearchResult search() { return ::chess::search(board, &history, search_params); }

        /**
         * @brief Pushes the current move to the history stack
         */
        inline void pushHistory(){ history.push(board, curr_move); }

        /**
         * @brief Makes a move, updating the board, the side to move and the move history
         * @attention User should call `generateMoves()` after calling this function
         */
        void make(Move& move) { ::make(move, board, &history); curr_move = move;}

        /**
         * @brief Unmakes current move, restoring the board to the previous state, 
         * may be called only once after `make()`
         * @attention User should call `generateMoves()` after calling this function
         */
        inline void unmake() { ::unmake(curr_move, board, &history); }

        /**
         * @brief Sets the search parameters for the search function
         */
        inline void setSearchParams(SearchParams params) { search_params = params; }

        GameHistory history;
        MoveList move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        GameState state;  
        SearchParams search_params;
    };
}