#pragma once

#include <random>
#include <limits>

#include "types.h"

namespace chess
{
    // Zobrist hash keys
    class Zobrist
    {
    public:
        static Hash hash_pieces[2][6][64];
        static Hash hash_castling[16];
        static Hash hash_turn;
        static Hash hash_enpassant[8];
    };

    void init_hashing();
}