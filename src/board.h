#pragma once

#include "pieces.h"

namespace chess{
    class Board
    {
    public:
        static const int mailbox[64];
        static const int mailbox64[120];

        Board() = default;
        
        const Board& init();

        int* board;
        int operator[](int index) {return this->board[index]; };
    };
}
