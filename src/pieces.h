#pragma once

#include "types.h"

namespace chess
{
    /*
        ### Piece representation

        Bits alocation:
            - bits 0-3 - piece type
            - bits 3-5 - color

    */ 
    namespace Piece{
        constexpr auto pieceMask = 7;  // 0b00111
        constexpr auto colorMask = 24; // 0b11000

        enum Type
        {
            // non sliding pieces (0b0xx)
            Empty = 0,
            Pawn = 1,
            Knight = 2,
            King = 3,

            // sliding pieces (0b1xx)
            Bishop = 4, 
            Rook = 5,
            Queen = 6,
        };

        enum Color
        {
            White = 8, //  0b01000
            Black = 16, // 0b10000
        };

        inline int getColor(const int& piece){
            return piece & colorMask;
        }

        inline int getType(const int& piece){
            return piece & pieceMask;
        }
    }
}


