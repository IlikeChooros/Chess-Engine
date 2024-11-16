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
    int evaluate(Board* board, CacheMoveGen* c, MoveList* ml);

    // TODO: delete this
    GameStatus get_status(Board* board, GameHistory* gh, MoveList *ml);
    GameStatus get_status(Board* board, MoveList *ml);
    GameStatus get_status(Board& board);
    
    std::string game_status_to_string(GameStatus status);

    extern const int piece_values[6];
}