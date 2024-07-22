#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>

#include "board.h"
#include "move.h"
#include "utils.h"

namespace chess
{

    class Manager
    {
    public:
        typedef unsigned long uint64_t;
        typedef unsigned int uint32_t;
        typedef struct {
            uint32_t x, y;
            uint32_t flags;
        } PieceMoveInfo;

        static uint64_t attacks_to[2][64];
        static uint64_t attacks_from[2][64];
        static const int castling_rights[2][4];
        static const int castling_offsets[2][2];
        static const int castling_flags[2];

        Manager(Board* board = nullptr);
        Manager& operator=(Manager&& other);

        bool movePiece(uint32_t from, uint32_t to);
        std::list<PieceMoveInfo> getPieceMoves(uint32_t from);
        int getSide();
        int generateMoves(bool validate = true);
        bool validateMove(Move& move, int king_pos, bool is_white);

    
        void make(Move& move, bool validate = true);
        void unmake();
        void generatePseudoLegalMoves(int& n_moves, int* move_list);
        void addMove(int from, int to, int flags, int* move_list, int& n_moves);
        void addAttack(int from, int to, bool piece_is_white);
        void handleCapture(Move& move);
        void handleMove(Move& move);
        void checkKingCastling(bool is_white, int j, int king_index);
        void handleCastlingMove(bool is_king_castle, int from, int to);

        int side;
        std::vector<int> move_list;
        std::vector<int> prev_move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        Move prev_move;
        int captured_piece;
        int prev_captured_piece;
        int black_king_pos;
        int white_king_pos;
        int halfmove_clock;
        int fullmove_counter;
    };
}