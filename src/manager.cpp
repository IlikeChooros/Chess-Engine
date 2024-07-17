#include "manager.h"

namespace chess
{
    uint64_t Manager::attacks_to[2][64] = {};
    uint64_t Manager::attacks_from[2][64] = {};

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
     * @brief Moves a piece from one square to another, if the move is valid
     * it updates the board, calls move generation and changes the side to move
     * @param from The square to move the piece from (as an index)
     * @param to The square to move the piece to (as an index)
     * @return True if the move was successful, false otherwise
     */
    bool Manager::movePiece(uint32_t from, uint32_t to)
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

        dlogf("Invalid move: from %s to %s\n",
                square_to_str(from).c_str(),
                square_to_str(to).c_str()
        );
        return false;
    }

    std::list<Manager::PieceMoveInfo> Manager::getPieceMoves(uint32_t from){
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
        bool is_white = this->side == Piece::White;

        for(int i = 0; i < 64; i++){
            // reset the attacks_from array & attacks_to bitboards
            attacks_from[is_white][i] = 0; 
            attacks_to[is_white][i] = 0;
            if(iboard[i] == Piece::Empty || Piece::getColor(iboard[i]) != this->side)
                continue;
            
            int type = Piece::getType(iboard[i]);
            

            if (type != Piece::Pawn){
                dlogf("Generating moves for %s at %s\n", 
                    Piece::toStr(iboard[i]).c_str(), 
                    square_to_str(i).c_str()
                );
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

                        // Update attcked squares
                        attacks_to[is_white][n] |= 1 << i; // this creates a bitboard with the squares that the piece attacks to
                        attacks_from[is_white][i] |= 1 << n; // this creates a bitboard with the squares that the piece attacks from
                        // basically, attacks_to[is_white][x] is usefull if you want to check how many pieces are attacking 
                        // given square x, and attacks_from[is_white][x] if you want to check how many squares are being attacked
                        // by the piece in square x. The `is_white` variable is used to differentiate between white and black pieces
                        
                        dlogf("Attacks to: %s (%d)\n", square_to_str(n).c_str(), n);

                        // If the square is not empty
                        if (iboard[n] != Piece::Empty){
                            if (Piece::getColor(iboard[n]) != this->side){
                                dlogf(
                                    "Added pseudo-legal capture\n"
                                    "Capture by %s from %s to %s\n",
                                    Piece::toStr(iboard[i]).c_str(), 
                                    square_to_str(i).c_str(),
                                    square_to_str(n).c_str()
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

                dlog("Attacks from: \n");
                dbitboard(attacks_from[is_white][i]);
            } else {
                // Pawn moves

            }
        }

    // #if DEBUG_DETAILS
    //     bool is_white = this->side == Piece::White;
    //     for(int i = 0; i < 64; i++){
    //         dlogf("Attacks to square %c%d\n", 'A' + i%8, 8 - i/8);
    //         dbitboard(attacks_to[!is_white][i]);
    //         dlogf("Attacks from square %c%d\n", 'A' + i%8, 8 - i/8);
    //         dbitboard(attacks_from[is_white][i]);
    //     }
    // #endif

        return n_moves;
    }

    void Manager::addMove(int from, int to, int flags){
        move_list[n_moves++] = int(Move(from, to, flags));
    }
}