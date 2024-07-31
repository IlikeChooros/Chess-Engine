#pragma once

#include <string>
#include "types.h"
#include "utils.h"


namespace chess
{
    /*
        ### Piece representation

        Bits alocation:
            - bits 1-3 - piece type
            - bits 4-5 - color
            - bits 6+  - special moves
    */ 
    class Piece
    {
        public:
        static const int pieceMask = 0b00111;  // 0b00111
        static const int colorMask = 0b11000; // 0b11000
        static const int bits = 5;

        // Piece types
        enum Type
        {
            // non sliding pieces (0b000xx)
            Empty = 0,
            Pawn = 1,
            Knight = 2,
            King = 3,

            // sliding pieces (0b001xx)
            Bishop = 4, // 0b00100
            Rook = 5,  // 0b00101
            Queen = 6, // 0b00110
        };

        // Piece colors
        enum Color
        {
            White = 8,  // 0b01000
            Black = 16, // 0b10000
        };

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
         * @brief Check if the piece is white
         */
        static inline bool isWhite(const int& piece){
            return getColor(piece) == White;
        }
        /**
         * @brief Create a piece with a given type and color
         */
        static inline int createPiece(int type, int color, int special = 0){
            return type | color | special;
        }

        /**
         * @brief Check if the piece is a sliding piece
         */
        static inline bool isSliding(const int& piece){
            return getType(piece) & 0b100;
        }

        /**
         * @brief Get the piece representing the king of a given color
         */
        static inline int getKing(int color){
            return King | color;
        }

        /**
         * @brief Get the piece representing the rook of a given color
         */
        static inline int getRook(int color){
            return Rook | color;
        }

        static std::string notation(int piece, int square){
            std::string str = "";
            if (piece == Empty)
                return " ";

            switch (piece & pieceMask)
            {
                case Knight:
                    str += "N";
                    break;
                case King:
                    str += "K";
                    break;
                case Bishop:
                    str += "B";
                    break;
                case Rook:
                    str += "R";
                    break;
                case Queen:
                    str += "Q";
                    break;
                default:
                    break;
            }
            return str + square_to_str(square);
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
    };
}


