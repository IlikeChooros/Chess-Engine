#include "manager.h"


namespace chess
{
    constexpr int WHITE_KING_START = 60, BLACK_KING_START = 4;

    uint64_t Manager::attacks_to[2][64] = {};
    uint64_t Manager::attacks_from[2][64] = {};
    int Manager::king_pos[2] = {};

    Manager::Manager(Board* board)
    {
        this->board = board;
        this->n_moves = 0;
        this->move_list = std::make_unique<int[]>(256);
        this->side = Piece::White;
        if (board != nullptr){
            king_pos[0] = board->findPiece(Piece::King, Piece::Black);
            king_pos[1] = board->findPiece(Piece::King, Piece::White);
            generateMoves();
        }
            
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
                handleMove(last_move);
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
        int pseudo_moves[256] = {};
        int n_pseudo_moves = 0;
        
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
                                    "Added pseudo-legal capture "
                                    "by %s from %s to %s\n",
                                    Piece::toStr(iboard[i]).c_str(), 
                                    square_to_str(i).c_str(),
                                    square_to_str(n).c_str()
                                );
                                addMove(i, n, Move::FLAG_CAPTURE, pseudo_moves, n_pseudo_moves);
                            }
                            break;
                        }
                        // move
                        addMove(i, n, Move::FLAG_NONE, pseudo_moves, n_pseudo_moves);
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
                        addMove(i, n, Move::FLAG_CAPTURE, pseudo_moves, n_pseudo_moves);
                        dlogf("Pawn capture to %s\n", square_to_str(n).c_str());
                    } else {
                        // Check if enpassant is possible, if the last move was a double pawn move and there is a pawn
                        // in the correct position (next to the attacking pawn, n + previous_pawn_offset[0])
                        if (last_move.isDoubleMove() && int(last_move.getTo()) == Board::mailbox[Board::mailbox64[n] + Board::pawn_move_offsets[!is_white][0]]){
                            addMove(i, n, Move::FLAG_ENPASSANT_CAPTURE, pseudo_moves, n_pseudo_moves);
                            dlogf("En passant to %s\n", square_to_str(n).c_str());
                        }
                    }
                }

                // Check for pawn moves
                int n = Board::mailbox[Board::mailbox64[i] + Board::pawn_move_offsets[is_white][0]];
                if (n == -1 || iboard[n] != Piece::Empty) 
                    continue; // Out of the board or piece in the way

                addMove(i, n, Move::FLAG_NONE, pseudo_moves, n_pseudo_moves);
                dlogf("Pawn move to %s\n", square_to_str(n).c_str());

                if (i / 8 == start_rank){
                    // Check for double pawn moves
                    n = Board::mailbox[Board::mailbox64[i] + Board::pawn_move_offsets[is_white][1]];
                    if (iboard[n] != Piece::Empty) // Piece in the way
                        continue;

                    addMove(i, n, Move::FLAG_DOUBLE_PAWN, pseudo_moves, n_pseudo_moves);
                    dlogf("Pawn double move to %s\n", square_to_str(n).c_str());
                }
            }
        }


        dlogln("Checking castle rights");
        // Check for castling
        const int castling_rights[2][4] = {
            {BLACK_KING_START,  0,  7, Piece::Black}, // Black (index 0 -> is_white = false)
            {WHITE_KING_START, 56, 63, Piece::White} // White
        };
        const int castling_offsets[2][2] = {
            { -2, 2}, // target relative position (king + offset -> target position)
            { -1, 1}, // position offset (pos += offset while pos != king)
        };
        const int castle_flags[2] = {
            Move::FLAG_QUEEN_CASTLE, Move::FLAG_KING_CASTLE
        };
        for(int i = 0; i < 2; i++){
            // Get starting position for the king
            int king = castling_rights[i][0]; // 0 black, 1 white
            if (iboard[king] != Piece::getCastleKing(castling_rights[i][3])) // King has moved
                continue;

            dlogf("Found valid king at %s\n", square_to_str(king).c_str());
            // Check if rooks have moved / still have castling rights
            for(int j = 0; j < 2; j++){
                int rook = iboard[castling_rights[i][j + 1]];
                if (rook != Piece::getCastleRook(castling_rights[i][3])) // not the correct piece / doesn't have castling rights
                    continue;
                
                dlogf("Valid rook at %s\n", square_to_str(castling_rights[i][j + 1]).c_str());
                if (attacks_to[!i][king] != 0) // king at check
                    continue;
                // Check if the squares between the king and the rook are empty and 
                // not attacked by the enemy color
                int pos = king + castling_offsets[1][j];
                int target = king + castling_offsets[0][j];

                while(pos != target){
                    dlogf("Checking position at %s\n", square_to_str(pos).c_str());
                    if (iboard[pos] != Piece::Empty || attacks_to[!i][pos] != 0) 
                        break;
                    pos += castling_offsets[1][j];
                }
                // valid castle
                if (pos == target){
                    dlogf("Added possible castle at %s\n", square_to_str(target).c_str());
                    addMove(king, target, castle_flags[j], move_list.get(), n_moves);
                }
            }
        }

    #if DEBUG_DETAILS
        // for(int i = 0; i < 64; i++){
        //     if (iboard[i] == Piece::Empty)
        //         continue;
        //     dlogf("Piece at %s: %s\n", square_to_str(i).c_str(), Piece::toStr(iboard[i]).c_str());
        //     dlogf("Attacks to square %s\n", square_to_str(i).c_str());
        //     dbitboard(attacks_to[0][i] | attacks_to[1][i]);
        //     dlogf("Attacks from square %s\n", square_to_str(i).c_str());
        //     dbitboard(attacks_from[0][i] | attacks_from[1][i]);
        // }
    #endif

        // validate pseudo moves
        for(int i = 0; i < n_pseudo_moves; i++){
            move_list[n_moves++] = pseudo_moves[i];
        }

        return n_moves;
    }

    /**
     * @brief Moves the piece to given position, it handles castling
     */
    void Manager::handleMove(Move& move){
        int* iboard = board->board.get();
        int from = move.getFrom(), to = move.getTo();

        // If that's a castle, move the corresponding rooks
        if (move.isQueenCastle()){
            int rook_from = Piece::getColor(iboard[from]) == Piece::White ? 56 : 0;
            iboard[to + 1] = Piece::deleteSpecial(iboard[rook_from], Piece::Castling);
            iboard[rook_from] = Piece::Empty;
            iboard[from] = Piece::deleteSpecial(iboard[from], Piece::Castling);
        }
        else if(move.isKingCastle()){
            int rook_from = Piece::getColor(iboard[from]) == Piece::White ? 63 : 7;
            iboard[to - 1] = Piece::deleteSpecial(iboard[rook_from], Piece::Castling);
            iboard[rook_from] = Piece::Empty;
            iboard[from] = Piece::deleteSpecial(iboard[from], Piece::Castling);
        }

        iboard[to] = iboard[from];
        iboard[from] = Piece::Empty;
    }

    /**
     * @brief If the move is a capture, it correctly updates the board
     * @attention Should be called before `handleMove()` as it won't update correctly `captured_piece`
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
    void Manager::addMove(int from, int to, int flags, int* move_list, int& n_moves){
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