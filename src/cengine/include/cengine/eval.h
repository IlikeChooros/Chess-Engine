#pragma once

#include "utils.h"
#include "board.h"
#include "history.h"
#include "cache.h"

namespace chess
{
    enum GameStatus
    {
        ONGOING,
        CHECKMATE,
        STALEMATE,
        DRAW
    };

    
    void init_eval();
    int evaluate(Board* board);
    GameStatus get_status(Board* board, GameHistory* gh, MoveList *ml);

    extern const int piece_values[6];
}