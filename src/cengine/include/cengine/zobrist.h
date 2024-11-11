#pragma once

#include "types.h"

namespace chess
{
    // Zobrist hash keys
    class Zobrist
    {
    public:
        static int hash_pieces[2][6][64];
        static int hash_castling[16];
        static int hash_turn;
        static int hash_enpassant[8];
    };
}