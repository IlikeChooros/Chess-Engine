#pragma once

#include "utils.h"
#include "board.h"
#include "cache.h"

namespace chess
{
    // Evaluation class
    class Eval
    {
    public:
        // Game phases
        constexpr static int 
            MIDDLE_GAME = 0, 
            ENDGAME = 1;
        
        // Endgame factors
        constexpr static int ENDGAME_FACTOR_PIECES[6]  = {0, 1, 0, 1, 4, 8};
        constexpr static int MAX_ENDGAME_FACTOR =
            2*(ENDGAME_FACTOR_PIECES[0]*8 + ENDGAME_FACTOR_PIECES[1] * 2 + ENDGAME_FACTOR_PIECES[2] +
            ENDGAME_FACTOR_PIECES[3] * 2 + ENDGAME_FACTOR_PIECES[4] * 2 + ENDGAME_FACTOR_PIECES[5]);
        
        // Piece values source: https://www.chessprogramming.org/Simplified_Evaluation_Function
        static constexpr int piece_values[6] = {100, 320, 20000, 330, 500, 900};

        // Bitboards for files
        static constexpr Bitboard file_bitboards[8] = {
            0x0101010101010101ULL,
            0x0202020202020202ULL,
            0x0404040404040404ULL,
            0x0808080808080808ULL,
            0x1010101010101010ULL,
            0x2020202020202020ULL,
            0x4040404040404040ULL,
            0x8080808080808080ULL,
        };

        static constexpr Bitboard rank_bitboards[8] = {
            0x00000000000000FFULL,
            0x000000000000FF00ULL,
            0x0000000000FF0000ULL,
            0x00000000FF000000ULL,
            0x000000FF00000000ULL,
            0x0000FF0000000000ULL,
            0x00FF000000000000ULL,
            0xFF00000000000000ULL,
        };

        // Manhatten distance of a squara
        static int8_t manhattan_distance[64][64];

        static const int white_piece_square_table[2][6][64];
        static int piece_square_table[2][2][6][64];
        static Bitboard passed_pawn_masks[2][64];
        static TTable<int> pawn_table;

        Eval() = delete;
        
        static void init();
        static int evaluate(Board& board);
    };
}