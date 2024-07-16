#pragma once

#include <memory>
#include "pieces.h"

namespace chess{
    class Board
    {
    public:
        static const int mailbox64[64];
        static const int mailbox[120];
        static const int piece_move_offsets[6][8];
        static const int n_piece_rays[6];
        static const bool is_piece_sliding[6];

        Board() = default;
        
        const Board& init();

        std::unique_ptr<int[]> board;

        /**
         * @brief Get the piece at a given index
         */
        int operator[](int index) {return this->board[index]; };
    };
}
