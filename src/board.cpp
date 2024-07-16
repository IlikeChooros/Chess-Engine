#include "board.h"


namespace chess
{

    /*
        Using mailbox approach to represent the board
    */

    const int Board::mailbox64[64] = {
        21, 22, 23, 24, 25, 26, 27, 28,
        31, 32, 33, 34, 35, 36, 37, 38,
        41, 42, 43, 44, 45, 46, 47, 48,
        51, 52, 53, 54, 55, 56, 57, 58,
        61, 62, 63, 64, 65, 66, 67, 68,
        71, 72, 73, 74, 75, 76, 77, 78,
        81, 82, 83, 84, 85, 86, 87, 88,
        91, 92, 93, 94, 95, 96, 97, 98,
    };

    const int Board::mailbox[120] = {
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
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    // Piece move offsets, access by Piece::Type - 1 (excluding the pawn)
    const int Board::piece_move_offsets[6][8] = {
        { 0,    0,   0,  0, 0,  0,  0,  0}, // Pawn (not used, Piece::Type = 1)
        {-21, -19, -12, -8, 8, 12, 19, 21}, // Knight (type = 2)
        {-11, -10,  -9, -1, 1,  9, 10, 11}, // King (type = 3)
        {-11,  -9,   9, 11, 0,  0,  0,  0}, // Bishop (type = 4)
        {-10,  -1,   1, 10, 0,  0,  0,  0}, // Rook (type = 5)
        {-11, -10,  -9, -1, 1,  9, 10, 11}, // Queen (type = 6)
    };

    // Number of piece rays, access by Piece::Type - 1 (excluding the pawn)
    const int Board::n_piece_rays[6] = {
        0, 8, 8, 4, 4, 8 // Empty (Pawn), Knight, King, Bishop, Rook, Queen
    };

    // Boolean, wheter the piece is sliding, access by Piece::Type - 1 (excluding the pawn)
    const bool Board::is_piece_sliding[6] = {
        false, false, false, true, true, true // Pawn, Knight, King, Bishop, Rook, Queen
    };

    /**
     * @brief Initialize the board with the default chess pieces
     */
    const Board& Board::init(){       

        board = std::unique_ptr<int[]>(new int[64]());

        for (auto i=0; i < 8; i++){
            board[i + 8] = Piece::createPiece(Piece::Pawn, Piece::White);
            board[i + 48] = Piece::createPiece(Piece::Pawn, Piece::Black); 
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

