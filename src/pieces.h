#pragma once

#include "types.h"

namespace chess
{
    namespace Piece{
        constexpr auto pieceMask = 7;
        constexpr auto colorMask = 24;

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
            White = 8,
            Black = 16,
        };


    }
}


