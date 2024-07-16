#include "manager.h"

namespace chess
{
    Manager::Manager(Board* board)
    {
        this->board = board;
        this->n_moves = 0;
        this->move_list = std::make_unique<int[]>(256);
        this->side = Piece::White;
        if (board != nullptr)
            generateMoves();
    }

    Manager& Manager::operator=(Manager&& other)
    {
        this->board = other.board;
        this->n_moves = other.n_moves;
        this->move_list = std::move(other.move_list);
        this->side = other.side;
        return *this;
    }

    /**
     * @brief Moves a piece from one square to another
     * @param from The square to move the piece from (as an index)
     * @param to The square to move the piece to (as an index)
     * @return True if the move was successful, false otherwise
     */
    bool Manager::movePiece(unsigned int from, unsigned int to)
    {
        int* iboard = this->board->board.get();
        if(iboard[from] == Piece::Empty || from == to || Piece::getColor(iboard[from]) != this->side)
            return false;

        for(int i = 0; i < n_moves; i++){
            auto move = Move(move_list[i]);
            if (move.getFrom() == from && move.getTo() == to){
                iboard[to] = iboard[from];
                iboard[from] = Piece::Empty;
                this->side ^= Piece::colorMask; // switch side
                generateMoves();
                return true;
            }
        }

        printf("Invalid move: %c%d to %c%d\n", 'A' + from%8, 1 + from/8, 'A' + to%8, 1 + to/8);

        return false;
    }

    std::list<Manager::PieceMoveInfo> Manager::getPieceMoves(unsigned int from){
        std::list<PieceMoveInfo> moves;
        for(int i = 0; i < n_moves; i++){
            auto move = Move(move_list[i]);
            if (move.getFrom() == from){
                moves.push_back({
                    .x = move.getTo() % 8,
                    .y = move.getTo() / 8,
                    .flags = move.getFlags()
                });
            }
        }
        return moves;
    }

    int Manager::generateMoves()
    {
        n_moves = 0;
        int* iboard = this->board->board.get();

        for(int i = 0; i < 64; i++){
            if(iboard[i] == Piece::Empty || Piece::getColor(iboard[i]) != this->side)
                continue;
            
            int type = Piece::getType(iboard[i]);

            if (type != Piece::Pawn){
                type--;
                // Use the piece move offsets
                for(int j = 0; j < Board::n_piece_rays[type]; j++){
                    for(int n = i;;){
                        // mailbox64 has indexes for the 64 valid squares in mailbox.
                        // If, by moving the piece, we go outside of the valid squares (n == -1),
                        // we break the loop. Else, the n has the index of the next square.
                        n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][j]];
                        if (n == -1){// outside of the board
                            break;
                        }
                        if (iboard[n] != Piece::Empty){
                            if (Piece::getColor(iboard[n]) != this->side){
                                printf(
                                    "Added pseudo-legal capture\n"
                                    "Capture by %s from %c%d to %c%d\n",
                                    Piece::toStr(iboard[i]).c_str(), 
                                    char('A' + i%8), 1 + i/8,
                                    char('A' + n%8), 1 + n/8
                                );
                                addMove(i, n, Move::FLAG_CAPTURE);
                            }
                            break;
                        }
                        // move
                        addMove(i, n, Move::FLAG_NONE);
                        // If the piece is not sliding, break the loop
                        if (!Board::is_piece_sliding[type]){
                            break;
                        }
                    }
                }
            } else {
                // Pawn moves

            }
        }
        return n_moves;
    }

    void Manager::addMove(int from, int to, int flags){
        move_list[n_moves++] = int(Move(from, to, flags));
    }
}