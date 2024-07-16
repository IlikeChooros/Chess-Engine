#pragma once

#include <memory>
#include <list>
#include "stdio.h"

#include "board.h"
#include "move.h"

namespace chess
{

    class Manager
    {
        void addMove(int from, int to, int flags);
        public:
        typedef struct {
            unsigned int x, y;
            unsigned int flags;
        } PieceMoveInfo;

        Manager(Board* board = nullptr);
        Manager& operator=(Manager&& other);

        bool movePiece(unsigned int from, unsigned int to);
        std::list<PieceMoveInfo> getPieceMoves(unsigned int from);
        int generateMoves();

        int side;
        std::unique_ptr<int[]> move_list;
        int n_moves;
        Board* board;
    };
}