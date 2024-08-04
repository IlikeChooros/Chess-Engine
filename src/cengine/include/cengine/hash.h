#pragma once

// Implementation of zobrist hashing for chess & transposition tables

#include <map>
#include <random>

#include "move.h"
#include "types.h"
#include "board.h"
#include "utils.h"

namespace chess
{
    void init_hashing();
    uint64_t get_hash(Board* board);
}