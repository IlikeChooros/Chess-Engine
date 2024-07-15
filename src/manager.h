#pragma once

#include "board.h"

namespace chess
{
    class Manager
    {
        public:
        Manager(Board* board);

        bool movePiece(int from, int to);

        Board* board;
    };
}