#pragma once

#include "pieces.h"
#include "types.h"

namespace chess
{   
    // Dataclass for mailbox representations
    class Mailbox
    {
    public:
        static const int mailbox64[64];
        static const int mailbox[120];
        static const int piece_move_offsets[6][8];
        static const int pawn_attack_offsets[2][2];
        static const int pawn_move_offsets[2][2];
        static const int n_piece_rays[6];

        static Bitboard mailboxAttacks(int type, Bitboard occupied, int square, bool is_sliding);
        static Bitboard mailboxPawnMoves(Bitboard occupied, int square, bool is_white);
        static Bitboard mailboxBishop(int square, Bitboard occupied);
        static Bitboard mailboxRook(int square, Bitboard occupied);

        static Bitboard rookMask(int square);
        static Bitboard bishopMask(int square);
    };
}