#pragma once


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
    };
}