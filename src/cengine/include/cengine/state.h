#pragma once

#include <vector>

#include "types.h"
#include "move.h"
#include "types.h"
#include "pieces.h"
#include "castling_rights.h"

namespace chess
{
    // Struct to store the state of the game
    typedef struct 
    {
        uint64_t hash; // 64 bits for Zobrist hash
        uint64_t move:Move::bits; // 16 bits
        uint64_t side_to_move:Piece::bits; // 5 bit for side to move (Piece::Color)
        uint64_t captured_piece:Piece::bits; // 5 bits for piece
        uint64_t enpassant_target:6; // 6 bits for square (0 - 63)
        uint64_t halfmove_clock:6; // 6 bits for halfmove clock (0 - 63 (max is 50))
        uint64_t fullmove_counter:13; // 13 bits for fullmove counter (0 - 8192)
        uint64_t castling_rights:CastlingRights::bits; // 4 bits for castling rights
        uint64_t reserved:9;
        // That gives total of 128 bits, instead of 6*32 + 64 = 256 bits
    } State;

    // Vector of 'State' structs, representing the game history
    typedef std::vector<State> StateList;
}