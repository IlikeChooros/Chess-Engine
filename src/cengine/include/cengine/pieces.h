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

        static const int pieceMask = 0b00111;  // 0b00111
        static const int colorMask = 0b11000; // 0b11000
        static const int bits = 5;

        // Promotion pieces (without color), matching the `Move::getPromotionPiece()` method
        static constexpr int promotionPieces[4] = { Knight, Bishop, Rook, Queen };

        /**
         * @brief Get the color of a piece
         * @return piece & colorMask, either 0b01000 or 0b10000
         */
        static constexpr int getColor(const int& piece){
            return piece & colorMask;
        }

        /**
         * @brief Get the type of a piece
         */
        static constexpr int getType(const int& piece){
            return piece & pieceMask;
        }
        
        /**
         * @brief Check if the piece is white
         */
        static constexpr bool isWhite(const int& piece){
            return getColor(piece) == White;
        }
        /**
         * @brief Create a piece with a given type and color
         */
        static constexpr int createPiece(int type, int color, int special = 0){
            return type | color | special;
        }

        /**
         * @brief Check if the piece is a sliding piece
         */
        static constexpr bool isSliding(const int& piece){
            return getType(piece) & 0b100;
        }

        /**
         * @brief Get the piece representing the king of a given color
         */
        static constexpr int getKing(int color){
            return King | color;
        }

        /**
         * @brief Get the piece representing the rook of a given color
         */
        static constexpr int getRook(int color){
            return Rook | color;
        }

        /**
         * @brief Return the opposite color
         */
        static constexpr int opposite(int color){
            return color ^ colorMask;
        }

        static int getPromotionPiece(char piece){
            switch (piece)
            {
                case 'q':
                    return Queen;
                case 'r':
                    return Rook;
                case 'b':
                    return Bishop;
                case 'n':
                    return Knight;
                default:
                    return -1;
            }
        }

        static char toChar(int piece, bool uselower = false){
            if (piece == Empty)
                return ' ';
            char piece_char = ' ';
            switch (piece & pieceMask)
            {
                case Pawn:
                    piece_char = 'P';
                    break;
                case Knight:
                    piece_char = 'N';
                    break;
                case King:
                    piece_char = 'K';
                    break;
                case Bishop:
                    piece_char = 'B';
                    break;
                case Rook:
                    piece_char = 'R';
                    break;
                case Queen:
                    piece_char = 'Q';
                    break;
            }

            if (uselower){
                piece_char = isWhite(piece) ? tolower(piece_char) : piece_char;
            }
            return piece_char;
        }
    };
}


