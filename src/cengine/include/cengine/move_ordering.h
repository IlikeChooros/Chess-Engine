#pragma once

#include <algorithm>

#include "eval.h"
#include "move.h"
#include "board.h"
#include "transp_table.h"

namespace chess
{
    void order_moves(MoveList *ml, MoveList *pv, Board *b, SearchCache* sc);
}