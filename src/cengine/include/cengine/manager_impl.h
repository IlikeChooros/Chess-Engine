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

namespace chess
{
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

        void init();
        void reload();
        int generateMoves();
        void make(Move& move);
        void unmake();
        GameState evalState();
        void pushHistory();

        GameHistory history;
        std::vector<uint32_t> move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        int captured_piece;
        GameState state;  
    };
}