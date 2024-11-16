#pragma once

#include "utils.h"
#include "board.h"
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
    int evaluate(Board& board);

    GameStatus get_status(Board* board, MoveList *ml);
    GameStatus get_status(Board& board);
    
    std::string game_status_to_string(GameStatus status);

    extern const int piece_values[6];
}