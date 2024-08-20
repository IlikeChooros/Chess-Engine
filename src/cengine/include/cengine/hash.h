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
    class Zobrist
    {
    public:
        static uint64_t hash_values[64 * 12 + 16 + 8 + 1];
        
        static void init_hashing();
        static uint64_t get_hash(Board* board);
        static uint64_t get_pawn_hash(Board* board);
    };
}