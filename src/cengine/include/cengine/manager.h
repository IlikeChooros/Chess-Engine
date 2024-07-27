#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>
#include <cmath>

#include "board.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"

namespace chess
{

    struct _History
    {
        Move move;
        int captured_piece;
        int enpassant_target;
        int halfmove_clock;
        int castling_rights;
    };

    class Manager
    {
    public:
        typedef struct {
            uint32_t x, y;
            uint32_t flags;
        } PieceMoveInfo;

        static uint64_t attacks_to[2][64];
        static uint64_t attacks_from[2][64];
        static const int castling_data[2][4];
        static const int castling_offsets[2][2];
        static const int castling_flags[2];
        static uint64_t in_between[64][64];

        Manager(Board* board = nullptr);
        Manager(const Manager& other) = delete;
        Manager& operator=(Manager&& other);

        bool movePiece(uint32_t from, uint32_t to);
        std::list<PieceMoveInfo> getPieceMoves(uint32_t from);
        int generateMoves(bool validate = true);
        bool validateMove(Move& move, int king_pos, bool is_white, uint64_t pinnedbb);

    
        void make(Move& move, bool validate = true);
        void unmake();
        void generatePseudoLegalMoves(int& n_moves, int* move_list);
        void addMove(int from, int to, int flags, int* move_list, int& n_moves);
        void addAttack(int from, int to, bool piece_is_white);
        void handleCapture(Move& move);
        void handleMove(Move& move);
        void checkKingCastling(bool is_white, int j, int king_index);
        void handleCastlingMove(bool is_king_castle, int from, int to);
        uint64_t rookAttacks(uint64_t occupied, int square);
        uint64_t bishopAttacks(uint64_t occupied, int square);
        uint64_t xRayRookAttacks(uint64_t occupied, uint64_t blockers, int square);
        uint64_t xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int square);

        std::list<_History> history;
        std::vector<int> move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        Move prev_move;
        int captured_piece;
        int prev_captured_piece;
        int black_king_pos;
        int white_king_pos;
    };
}