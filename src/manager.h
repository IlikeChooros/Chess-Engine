#pragma once

#include <memory>
#include <list>
#include <stdio.h>

#include "board.h"
#include "move.h"
#include "utils.h"

namespace chess
{

    class Manager
    {
        void addMove(int from, int to, int flags);
        void addAttack(int from, int to, bool piece_is_white);
        void handleCapture(Move& move);
        public:
        typedef unsigned long uint64_t;
        typedef unsigned int uint32_t;
        typedef struct {
            uint32_t x, y;
            uint32_t flags;
        } PieceMoveInfo;

        static uint64_t attacks_to[2][64];
        static uint64_t attacks_from[2][64];

        Manager(Board* board = nullptr);
        Manager& operator=(Manager&& other);

        bool movePiece(uint32_t from, uint32_t to);
        std::list<PieceMoveInfo> getPieceMoves(uint32_t from);
        int getSide();
        int generateMoves();

        int side;
        std::unique_ptr<int[]> move_list;
        int n_moves;
        Board* board;
        Move last_move;
        int captured_piece;
    };
}