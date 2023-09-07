#include "board.h"

/*
    Using TSCP approach
*/

extern auto board = chess::Board().init();

namespace chess
{
    const Board& Board::init(){
        mailbox = new int8_t[120]{
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
            -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
            -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
            -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
            -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
            -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
            -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
            -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };

        mailbox64 = new int8_t[64]{
            21, 22, 23, 24, 25, 26, 27, 28,
            31, 32, 33, 34, 35, 36, 37, 38,
            41, 42, 43, 44, 45, 46, 47, 48,
            51, 52, 53, 54, 55, 56, 57, 58,
            61, 62, 63, 64, 65, 66, 67, 68,
            71, 72, 73, 74, 75, 76, 77, 78,
            81, 82, 83, 84, 85, 86, 87, 88,
            91, 92, 93, 94, 95, 96, 97, 98
        };

        board = new int[64]();

        for (auto i=0; i < 8; i++){
            board[i + 8] = Piece::Pawn | Piece::White;
            board[i + 48] = Piece::Pawn | Piece::Black; 
        }
        board[0] = Piece::Rook | Piece::White;
        board[7] = Piece::Rook | Piece::White;

        board[56] = Piece::Rook | Piece::Black;
        board[63] = Piece::Rook | Piece::Black;

        board[1] = Piece::Knight | Piece::White;
        board[6] = Piece::Knight | Piece::White;

        board[57] = Piece::Knight | Piece::Black;
        board[62] = Piece::Knight | Piece::Black;

        board[2] = Piece::Bishop | Piece::White;
        board[5] = Piece::Bishop | Piece::White;

        board[58] = Piece::Bishop | Piece::Black;
        board[61] = Piece::Bishop | Piece::Black;

        board[3] = Piece::Queen | Piece::White;
        board[4] = Piece::King | Piece::White;

        board[59] = Piece::Queen | Piece::Black;
        board[60] = Piece::King | Piece::Black;
        return *this;
    }


}

