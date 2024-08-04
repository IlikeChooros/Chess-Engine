#pragma once

#include "utils.h"
#include "board.h"

namespace chess
{
    void init_eval();
    int evaluate(Board* board);
}