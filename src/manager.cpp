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
                last_move = move;
                handleCapture(last_move);
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

    /**
     * @brief Generates all possible moves for the current board state
     * @return The number of moves generated
     */
    int Manager::generateMoves()
    {
        n_moves = 0;
        int* iboard = this->board->board.get();
        
        for(int i = 0; i < 64; i++){
            // reset the attacks_from array & attacks_to bitboards
            attacks_from[0][i] = 0; 
            attacks_from[1][i] = 0; 
            attacks_to[0][i] = 0;
            attacks_to[1][i] = 0;
        }

        for(int i = 0; i < 64; i++){
            if(iboard[i] == Piece::Empty)
                continue;
            
            int type = Piece::getType(iboard[i]);
            int piece_color = Piece::getColor(iboard[i]);
            bool is_white = piece_color == Piece::White;

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
                        
                        // Attack = possible move (these are pieces)
                        addAttack(i, n, is_white);
                        dlogf("Attacks to: %s (%d)\n", square_to_str(n).c_str(), n);

                        // If the square is not empty
                        if (iboard[n] != Piece::Empty){
                            if (Piece::getColor(iboard[n]) != piece_color){
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

                dlogf("Attacks from: %s \n", square_to_str(i).c_str());
                dbitboard(attacks_from[is_white][i]);
            } else {
                // Pawn moves
                int start_rank = is_white ? 6 : 1;
                
                // Check for pawn attacks
                for(int j = 0; j < 2; j++){

                    int n = Board::mailbox[Board::mailbox64[i] + Board::pawn_attack_offsets[is_white][j]];
                    if(n == -1) // Out of the board
                        continue;

                    addAttack(i, n, is_white); // Update attacks_to and attacks_from bitboards

                    // Normal capture
                    if (iboard[n] != Piece::Empty && Piece::getColor(iboard[n]) != piece_color){
                        addMove(i, n, Move::FLAG_CAPTURE);
                        dlogf("Pawn capture to %s\n", square_to_str(n).c_str());
                    } else {
                        // Check if enpassant is possible, if the last move was a double pawn move and there is a pawn
                        // in the correct position (next to the attacking pawn, n + previous_pawn_offset[0])
                        if (last_move.isDoubleMove() && int(last_move.getTo()) == Board::mailbox[Board::mailbox64[n] + Board::pawn_move_offsets[!is_white][0]]){
                            addMove(i, n, Move::FLAG_ENPASSANT_CAPTURE);
                            dlogf("En passant to %s\n", square_to_str(n).c_str());
                        }
                    }
                }

                // Check for pawn moves
                int n = Board::mailbox[Board::mailbox64[i] + Board::pawn_move_offsets[is_white][0]];
                if (n == -1 || iboard[n] != Piece::Empty) 
                    continue; // Out of the board or piece in the way

                addMove(i, n, Move::FLAG_NONE);
                dlogf("Pawn move to %s\n", square_to_str(n).c_str());

                if (i / 8 == start_rank){
                    // Check for double pawn moves
                    n = Board::mailbox[Board::mailbox64[i] + Board::pawn_move_offsets[is_white][1]];
                    if (iboard[n] != Piece::Empty) // Piece in the way
                        continue;

                    addMove(i, n, Move::FLAG_DOUBLE_PAWN);
                    dlogf("Pawn double move to %s\n", square_to_str(n).c_str());
                }
            }
        }

    #if DEBUG_DETAILS
        for(int i = 0; i < 64; i++){
            if (iboard[i] == Piece::Empty)
                continue;
            dlogf("Piece at %s: %s\n", square_to_str(i).c_str(), Piece::toStr(iboard[i]).c_str());
            dlogf("Attacks to square %s\n", square_to_str(i).c_str());
            dbitboard(attacks_to[0][i] | attacks_to[1][i]);
            dlogf("Attacks from square %s\n", square_to_str(i).c_str());
            dbitboard(attacks_from[0][i] | attacks_from[1][i]);
        }
    #endif

        return n_moves;
    }

    /**
     * @brief If the move is a capture, it correctly updates the board
     */
    void Manager::handleCapture(Move& move){
        if (!move.isCapture())
            return;
        int* iboard = this->board->board.get();
        int to = move.getTo();
        int from = move.getFrom();
        int offset = 0;

        if(move.isEnPassant()){
            offset = Piece::getColor(iboard[from]) == Piece::White ? 8 : -8;
            dlogf("En passant capture\n");
        }

        captured_piece = iboard[to + offset];
        iboard[to + offset] = Piece::Empty;
    }

    /**
     * @brief Adds a move to the move list
     */
    void Manager::addMove(int from, int to, int flags){
        move_list[n_moves++] = int(Move(from, to, flags));
    }

    /**
     * @brief Updates `attacks_to` and `attacks_from` bitboards
     * @param from The square from which the attack is coming
     * @param to The square being attacked
     * @param is_white True if the attacking piece is white, false otherwise
     */
    void Manager::addAttack(int from, int to, bool is_white){
        // basically, attacks_to[is_white][x] is usefull if you want to check how many pieces are attacking 
        // given square x, and attacks_from[is_white][x] if you want to check how many squares are being attacked
        // by the piece in square x. The `is_white` variable is used to differentiate between white and black pieces
        attacks_to[is_white][to] |= 1UL << from; 
        attacks_from[is_white][from] |= 1UL << to;
    }

    /**
     * @brief Get current side color
     * @return Piece::Color, either Piece::White or Piece::Black
     */
    int Manager::getSide(){
        return this->side;
    }
}