#pragma once

#include <string>
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
         * @return piece & colorMask, either 0b01000 or 0b10000
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

        /**
         * @brief Create a piece with a given type and color
         */
        static inline int createPiece(int type, int color){
            return type | color;
        }

        /**
         * @brief Check if the piece is a sliding piece
         */
        static inline bool isSliding(const int& piece){
            return getType(piece) & 0b100;
        }

        static std::string toStr(int piece){
            std::string str = "";
            if (piece == Empty)
                return "Empty";
            if (piece & White){
                str += "White ";
            } else {
                str += "Black ";
            }

            switch (piece & pieceMask)
            {
                case Pawn:
                    str += "Pawn";
                    break;
                case Knight:
                    str += "Knight";
                    break;
                case King:
                    str += "King";
                    break;
                case Bishop:
                    str += "Bishop";
                    break;
                case Rook:
                    str += "Rook";
                    break;
                case Queen:
                    str += "Queen";
                    break;
            }
            return str;
        }

        // Piece types
        enum Type
        {
            // non sliding pieces (0b000xx)
            Empty = 0,
            Pawn = 1,
            Knight = 2,
            King = 3,

            // sliding pieces (0b001xx)
            Bishop = 4, 
            Rook = 5,
            Queen = 6,
        };

        // Piece colors
        enum Color
        {
            White = 8, //  0b01000
            Black = 16, // 0b10000
        };
    };
}


