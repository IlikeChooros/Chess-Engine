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
        static constexpr int seed = 0x21376942;
        static constexpr int castling_rights_index = 64 * 12;
        static constexpr int enpassant_target_index = 64 * 12 + 16;
        static constexpr int turn_index = 64 * 12 + 16 + 8;

        static void init_hashing();
        static uint64_t get_hash(Board* board);

        // Set the hash of a board
        inline static void set_hash(Board* board) { 
            board->hash() = get_hash(board); 
        }

        inline static uint64_t get_pawn_hash(Board* board) {
            uint64_t hash = 0;
            for(int k = 0; k < 2; k++){
                uint64_t pawns = board->bitboards(k)[Board::PAWN_TYPE];
                while(pawns){
                    hash ^= hash_values[k * 6 * 64 + Board::PAWN_TYPE * 64 + pop_lsb1(pawns)];
                }
            }
            return hash;
        }

        // Modify the hash with a piece move
        static inline void modify_piece(Board* board, bool color, int piece, int square) {
            board->hash() ^= hash_values[color * 6 * 64 + piece * 64 + square];
        }

        // Modify the hash with a castling rights change
        static inline void modify_castling_rights(Board* board, int rights){
            board->hash() ^= hash_values[castling_rights_index + rights];
        }

        // Modify the hash with an enpassant target change
        static inline void modify_enpassant_target(Board* board, int target){
            if (target != 0)
                board->hash() ^= hash_values[enpassant_target_index + target % 8];
        }

        // Modify the hash with a turn change
        static inline void modify_turn(Board* board){
            board->hash() ^= hash_values[turn_index];
        }
    };
}