#include <cengine/mailbox.h>


namespace chess
{   
    // Mailbox64 representation, 64 elements, contains
    // indexes for the 64 valid squares in mailbox (with 120 elements)
    const int Mailbox::mailbox64[64] = {
        21, 22, 23, 24, 25, 26, 27, 28,
        31, 32, 33, 34, 35, 36, 37, 38,
        41, 42, 43, 44, 45, 46, 47, 48,
        51, 52, 53, 54, 55, 56, 57, 58,
        61, 62, 63, 64, 65, 66, 67, 68,
        71, 72, 73, 74, 75, 76, 77, 78,
        81, 82, 83, 84, 85, 86, 87, 88,
        91, 92, 93, 94, 95, 96, 97, 98,
    };

    // Mailbox representation, 120 elements, -1 for invalid squares
    const int Mailbox::mailbox[120] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
        -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
        -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
        -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
        -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
        -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
        -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
        -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    // Pawn attack offsets, access by Piece::Color == Piece::White ([0] -> Black, [1] -> White)
    const int Mailbox::pawn_attack_offsets[2][2] = {
        { 11,  9}, // Black 
        {-11, -9}, // White 
    };

    // Pawn move offsets, access by Piece::Color == Piece::White ([0] -> Black, [1] -> White)
    const int Mailbox::pawn_move_offsets[2][2] = {
        { 10,  20}, // Black 
        {-10, -20}, // White 
    };

    // Piece move offsets, access by Piece::Type - 1 (excluding the pawn)
    const int Mailbox::piece_move_offsets[6][8] = {
        { 0,    0,   0,  0, 0,  0,  0,  0}, // Pawn (not used, Piece::Type = 1)
        {-21, -19, -12, -8, 8, 12, 19, 21}, // Knight (type = 2)
        {-11, -10,  -9, -1, 1,  9, 10, 11}, // King (type = 3)
        {-11,  -9,   9, 11, 0,  0,  0,  0}, // Bishop (type = 4)
        {-10,  -1,   1, 10, 0,  0,  0,  0}, // Rook (type = 5)
        {-11, -10,  -9, -1, 1,  9, 10, 11}, // Queen (type = 6)
    };

    // Number of piece rays, access by Piece::Type - 1 (excluding the pawn)
    const int Mailbox::n_piece_rays[6] = {
        0, 8, 8, 4, 4, 8 // Empty (Pawn), Knight, King, Bishop, Rook, Queen
    };


    /**
     * @brief Generate attacks for a given piece type
     * 
     * @param type The type of the piece (Piece::Rook - 1, Piece::Bishop - 1, Piece::Queen - 1, Piece::Knight - 1, Piece::King - 1)
     * @param occupied The bitboard of the occupied squares
     * @param square The square of the piece
     */
    Bitboard Mailbox::mailboxAttacks(int type, Bitboard occupied, int square, bool is_sliding)
    {
        Bitboard attacks = 0;
        for(int j = 0; j < n_piece_rays[type]; j++){
            for(int n = square;;){
                // mailbox64 has indexes for the 64 valid squares in mailbox.
                // If, by moving the piece, we go outside of the valid squares (n == -1),
                // we break the loop. Else, the n has the index of the next square.
                n = mailbox[mailbox64[n] + piece_move_offsets[type][j]];
                if (n == -1){// outside of the board
                    break;
                }
                
                // Attack = possible move 
                attacks |= 1ULL << n;

                // If the square is not empty
                if (occupied & (1ULL << n) || !is_sliding)
                    break;

            }
        }
        return attacks;
    }

    /**
     * @brief Generate pawn attacks for a given square
     */
    Bitboard Mailbox::mailboxPawnMoves(Bitboard occupied, int square, bool is_white)
    {
        const int ranks[2]   = {1, 6};
        bool is_special_rank = (square >> 3) == ranks[is_white];
        Bitboard moves       = 0;

        for(int j = 0; j < 2; j++)
        {
            int n = mailbox[mailbox64[square] + pawn_move_offsets[is_white][j]];

            // Check if the square is outside of the board or if it's occupied
            if (n == -1 || occupied & (1ULL << n))
                break;
            
            moves |= 1ULL << n;

            // Break if the pawn is not on the 2nd (white) or 7th (black) rank
            if(!is_special_rank)
                break;
        }
        return moves;
    }

    /**
     * @brief Generate bishop attacks for a given square
     */
    Bitboard Mailbox::mailboxBishop(int square, Bitboard occupied)
    {
        return mailboxAttacks(Piece::Bishop - 1, occupied, square, true);
    }

    /**
     * @brief Generate rook attacks for a given square
     */
    Bitboard Mailbox::mailboxRook(int square, Bitboard occupied)
    {
        return mailboxAttacks(Piece::Rook - 1, occupied, square, true);
    }

    /**
     * @brief Generate bishop mask for a given square
     */
    Bitboard Mailbox::bishopMask(int square)
    {
        Bitboard result = 0ULL;
        int rk = square / 8, fl = square % 8, r, f;
        for(r = rk + 1, f = fl + 1; r <= 6 && f <= 6; r++, f++) result |= (1ULL << (f + r * 8));
        for(r = rk + 1, f = fl - 1; r <= 6 && f >= 1; r++, f--) result |= (1ULL << (f + r * 8));
        for(r = rk - 1, f = fl + 1; r >= 1 && f <= 6; r--, f++) result |= (1ULL << (f + r * 8));
        for(r = rk - 1, f = fl - 1; r >= 1 && f >= 1; r--, f--) result |= (1ULL << (f + r * 8));
        return result;
    }

    /**
     * @brief Generate rook mask for a given square
     */
    Bitboard Mailbox::rookMask(int square)
    {
        Bitboard result = 0ULL;
        int rk = square / 8, fl = square % 8, r, f;
        for(r = rk + 1; r <= 6; r++) result |= (1ULL << (fl + r * 8));
        for(r = rk - 1; r >= 1; r--) result |= (1ULL << (fl + r * 8));
        for(f = fl + 1; f <= 6; f++) result |= (1ULL << (f + rk * 8));
        for(f = fl - 1; f >= 1; f--) result |= (1ULL << (f + rk * 8));
        return result;
    }
};