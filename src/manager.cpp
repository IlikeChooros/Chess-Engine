#include "manager.h"

namespace chess
{
    Manager::Manager(Board* board)
    {
        this->board = board;
    }

    bool Manager::movePiece(int from, int to)
    {
        if(this->board->board[from] == Piece::Empty)
            return false;

        this->board->board[to] = this->board->board[from];
        this->board->board[from] = Piece::Empty;

        return true;
    }
}