#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <chrono>

#include "board.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"

namespace chess
{

    class MoveList
    {
        public:
        typedef uint32_t move_t;

        MoveList(): n_moves(0) {}
        MoveList(const MoveList& other){
            n_moves = other.n_moves;
            memcpy(moves, other.moves, other.n_moves * sizeof(move_t));
        }
        MoveList& operator=(const MoveList& other){
            n_moves = other.n_moves;
            memcpy(moves, other.moves, other.n_moves * sizeof(move_t));
            return *this;
        }

        inline size_t size() const {return n_moves;}
        inline void add(move_t move) {moves[n_moves++] = move;}
        inline void clear() {n_moves = 0;}
        inline move_t& operator[](size_t i) {return moves[i];}
        inline move_t* begin() {return moves;}
        inline move_t* end() {return moves + n_moves;}

        move_t moves[256];
        size_t n_moves;
    };

    // Struct to store history of moves
    struct CHistory
    {
        uint32_t move:Move::bits; // 16 bits
        uint32_t side_to_move:Piece::bits; // 5 bit for side to move (Piece::Color)
        uint32_t captured_piece:Piece::bits; // 5 bits for piece
        uint32_t enpassant_target:6; // 7 bits for square (0 - 63)
        uint32_t halfmove_clock:6; // 6 bits for halfmove clock (0 - 63 (max is 50))
        uint32_t fullmove_counter:10; // 11 bits for fullmove counter (0 - 1023)
        uint32_t castling_rights:CastlingRights::bits; // 3 bits for castling rights
        uint32_t game_state:2; // 2 bits for game state
        uint32_t reserved:11;
        // That gives total of 64 bits, instead of 6*32 = 192 bits
    };

    class ManagerImpl
    {
    public:

        static uint64_t attacks_to[2][64];
        static uint64_t attacks_from[2][64];
        static const int castling_data[2][4];
        static const int castling_offsets[2][2];
        static const int castling_flags[2];
        static uint64_t in_between[64][64];
        static uint64_t pawnAttacks[2][64];
        static uint64_t knightAttacks[64];
        static uint64_t kingAttacks[64];

        typedef enum{
            Normal = 0,
            Checkmate = 1,
            Draw = 2,
            Repetition = 3,
        } GameState;

        ManagerImpl(Board* board = nullptr);
        ManagerImpl(const ManagerImpl& other) = delete;
        ManagerImpl& operator=(ManagerImpl&& other);

        void init();
        void reload();
        int generateMoves();
        void make(Move& move);
        void unmake();
        GameState evalState();
        int generatePseudoMoves(bool is_white, uint64_t occupied, uint64_t enemy_pieces, int* move_list);
        bool validateMove(Move& move, int king_pos, bool is_white, uint64_t pinnedbb, uint64_t pinners, uint64_t in_between_bb);
        void addMove(int from, int to, int flags, int* move_list, int& n_moves);
        void addAttack(int from, int to, bool piece_is_white);
        void handleCapture(Move& move);
        void handleMove(Move& move);
        void checkKingCastling(bool is_white, int j, int king_index);
        void handleCastlingMove(bool is_king_castle, int from, int to);
        void pushHistory();
        uint64_t rookAttacks(uint64_t occupied, int square);
        uint64_t bishopAttacks(uint64_t occupied, int square);
        uint64_t xRayRookAttacks(uint64_t occupied, uint64_t blockers, int square);
        uint64_t xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int square);
        

        size_t gen_legal_moves(MoveList* moves);
        void validate_castling_rights();

        /**
         * @brief Pop the least significant bit that is 1 from a bitboard and return its index
         */
        inline int pop_lsb1(uint64_t& bitboard) 
        {
            int lsb1 = bitScanForward(bitboard);
            bitboard &= (bitboard - 1);
            return lsb1;
        }

        std::list<CHistory> history;
        std::vector<int> move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        Move prev_move;
        int captured_piece;
        GameState state;  
    };
}