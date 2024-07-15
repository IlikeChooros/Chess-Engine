#pragma once

#include "types.h"

namespace chess
{
    /*
        ### Piece representation

        Bits alocation:
            - bits 1-3 - piece type
            - bits 4-5 - color

    */ 
    class Piece
    {
        public:
        static const int pieceMask = 7;  // 0b00111
        static const int colorMask = 24; // 0b11000

        /**
         * @brief Get the color of a piece
         */
        static inline int getColor(const int& piece){
            return piece & colorMask;
        }

        /**
         * @brief Get the type of a piece
         */
        static inline int getType(const int& piece){
            return piece & pieceMask;
        }

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

        
    };
}


