#pragma once

#include "pieces.h"

namespace chess{
    class Board
    {
    public:
        Board() = default;

        const Board& init();


        int* board;

        int8_t* mailbox;
        int8_t* mailbox64;

        int operator[](int index) {return this->board[index]; };
    };
}
