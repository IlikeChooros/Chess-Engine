#include <cengine/manager.h>

namespace chess
{
    constexpr int WHITE_KING_START = 60, BLACK_KING_START = 4;

    uint64_t Manager::attacks_to[2][64] = {};
    uint64_t Manager::attacks_from[2][64] = {};

    const int Manager::castling_data[2][4] = {
        {BLACK_KING_START,  0,  7, Piece::Black}, // Black (index 0 -> is_white = false)
        {WHITE_KING_START, 56, 63, Piece::White} // White
    };

    const int Manager::castling_offsets[2][2] = {
        { -2, 2}, // target relative position (king + offset -> target position)
        { -1, 1}, // position offset (pos += offset while pos != king)
    };

    const int Manager::castling_flags[2] = {
        Move::FLAG_QUEEN_CASTLE, Move::FLAG_KING_CASTLE
    };

    Manager::Manager(Board* board)
    {
        this->captured_piece = Piece::Empty;
        this->prev_captured_piece = Piece::Empty;
        this->n_moves = 0;
        this->move_list = std::vector<int>(256, 0);

        // load data from the board
        this->board = board;

        if (board == nullptr)
            return;

        generateMoves();
    }

    Manager& Manager::operator=(Manager&& other)
    {
        this->board = other.board;
        this->n_moves = other.n_moves;
        this->move_list = std::move(other.move_list);
        this->captured_piece = other.captured_piece;
        this->prev_captured_piece = other.prev_captured_piece;
        this->curr_move = other.curr_move;
        this->prev_move = other.prev_move;
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
        if(iboard[from] == Piece::Empty || from == to || Piece::getColor(iboard[from]) != this->board->m_side)
            return false;

        for(int i = 0; i < n_moves; i++){
            auto move = Move(move_list[i]);
            if (move.getFrom() == from && move.getTo() == to){
                make(move);
                return true;
            }
        }

        dlogf("Invalid move: from %s to %s\n",
                square_to_str(from).c_str(),
                square_to_str(to).c_str()
        );
        return false;
    }

    /**
     * @brief Get all possible moves for a piece at a given square
     */
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
     * @brief Makes a move, updating the board, the side to move and generating new moves
     */
    void Manager::make(Move& move, bool validate){
        if(this->board->m_side == Piece::Black){
            this->board->fullmoveCounter()++;
        }
        prev_move = curr_move;
        curr_move = move;

        // Update halfmove clock, if the move is a pawn move or a capture, reset the clock
        // (`handleCapture()` will update the clock if the move is a capture)
        if (Piece::getType((*board)[curr_move.getFrom()]) == Piece::Pawn){
            this->board->halfmoveClock() = 0;
        } else {
            this->board->halfmoveClock()++;
        }

        handleCapture(move);
        handleMove(move);
        this->board->m_side ^= Piece::colorMask;
        generateMoves(validate);
    }

    /**
     * @brief Unmakes current move, restoring the board to the previous state, 
     * may be called only once after `make()`
     */
    void Manager::unmake(){
        if(!curr_move && curr_move != prev_move)
            return;

        int* iboard = board->board.get();
        int from = curr_move.getFrom(), to = curr_move.getTo(), captured_pos = to;
        bool is_white = Piece::getColor(iboard[to]) == Piece::White;

        const int castling_rights[2][2] = {
            {CastlingRights::BLACK_QUEEN, CastlingRights::WHITE_QUEEN}, // queen castle
            {CastlingRights::BLACK_KING, CastlingRights::WHITE_KING} // king castle
        };

        if (curr_move.isCapture()){
            int offset = 0;
            if(curr_move.isEnPassant()){
                offset = is_white ? -8 : 8;
            }
            captured_pos += offset;
        } else if (curr_move.isQueenCastle()){
            // Get rook starting position
            int rook_from = is_white ? 56 : 0;
            iboard[rook_from] = iboard[to + 1]; // move the rook back
            iboard[to + 1] = Piece::Empty; // empty the rook's previous position
            board->m_castling_rights.add(castling_rights[1][is_white]);
        } else if (curr_move.isKingCastle()){
            int rook_from = is_white ? 63 : 7; 
            iboard[rook_from] = iboard[to - 1];
            iboard[to - 1] = Piece::Empty;
            board->m_castling_rights.add(castling_rights[0][is_white]);
        }

        iboard[from] = iboard[to];
        iboard[to] = Piece::Empty;
        iboard[captured_pos] = captured_piece;

        // restore other variables
        this->board->m_side ^= Piece::colorMask;
        this->captured_piece = prev_captured_piece;
        this->curr_move = prev_move;
    }

    /**
     * @brief Moves the piece to given position, it handles castling
     */
    void Manager::handleMove(Move& move){
        
        // Reset enpassant target every move
        board->m_enpassant_target = -1;
        int* iboard = board->board.get();        
        int from = move.getFrom(), to = move.getTo();
        bool is_white = Piece::getColor(iboard[from]) == Piece::White;

        // If that's a castle, move the corresponding rooks
        if (move.isCastle()){
            handleCastlingMove(move.isKingCastle(), from, to);
        }

        // If that's a double pawn move, set the enpassant target
        if (move.isDoubleMove()){
            int direction = is_white ? 8 : -8;
            board->m_enpassant_target = to + direction;
        }

        // Update castling rights
        if (Piece::getType(iboard[from]) == Piece::King){
            board->m_castling_rights.remove(Piece::getColor(iboard[from]) == Piece::White ? CastlingRights::WHITE : CastlingRights::BLACK);
        } else if (Piece::getType(iboard[from]) == Piece::Rook){
            // If the rook is moved from the starting position, remove the castling rights for that side
            if (from == 0 || from == 56){ // queen side rook
                board->m_castling_rights.remove(is_white ? CastlingRights::WHITE_QUEEN : CastlingRights::BLACK_QUEEN);
            } else if (from == 7 || from == 63){ // king side rook
                board->m_castling_rights.remove(is_white ? CastlingRights::WHITE_KING : CastlingRights::BLACK_KING);
            }
        }

        iboard[to] = iboard[from];
        iboard[from] = Piece::Empty;
    }

    /**
     * @brief Moves the rook to the correct position after a castling move
     * @param is_king_castle True if it's a king castle, false otherwise
     * @param from The starting position of the king
     * @param to The target position of the king
     */
    void Manager::handleCastlingMove(bool is_king_castle, int from, int to){
        int* iboard = board->board.get();
        int rook_from, rook_to;
        bool is_white = Piece::getColor(iboard[from]) == Piece::White;

        const int castling_pos[2][2] = {
            {0, 56}, // queen castle
            {7, 63} // king castle
        };

        if(is_king_castle){
            // get rook starting position
            rook_from = castling_pos[1][is_white];
            rook_to = to - 1; // rook target position
        } else {
            rook_from = castling_pos[0][is_white];
            rook_to = to + 1;
        }

        // Update castling rights
        board->m_castling_rights.remove(is_white ? CastlingRights::WHITE : CastlingRights::BLACK);

        // Move the rook (king will be handled in `handleMove()`)
        iboard[rook_to] = iboard[rook_from]; 
        iboard[rook_from] = Piece::Empty;
    }

    /**
     * @brief If the move is a capture, it correctly updates the board
     * @attention Should be called before `handleMove()` as it won't update correctly `captured_piece`
     */
    void Manager::handleCapture(Move& move){
        if (!move.isCapture()){
            captured_piece = Piece::Empty;
            return;
        }            

        board->m_halfmove_clock = 0;
        int* iboard = this->board->board.get();
        int to = move.getTo();
        int from = move.getFrom();
        int offset = 0;

        if(move.isEnPassant()){
            offset = Piece::getColor(iboard[from]) == Piece::White ? 8 : -8;
            dlogf("En passant capture\n");
        }

        prev_captured_piece = captured_piece;
        captured_piece = iboard[to + offset];
        iboard[to + offset] = Piece::Empty;
    }

    /**
     * @brief Generates all possible moves for the current board state
     * @return The number of moves generated
     */
    int Manager::generateMoves(bool validate)
    {
        this->n_moves = 0;
        int* iboard = this->board->board.get();

        int n_pseudo_moves = 0;
        std::unique_ptr<int[]> pseudo_moves(new int[256]);
        generatePseudoLegalMoves(n_pseudo_moves, pseudo_moves.get());

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

        if (!validate){
            n_moves = n_pseudo_moves;
            for(int i = 0; i < n_pseudo_moves; i++){
                move_list[n_moves++] = pseudo_moves[i];
            }
        } else {
            // validate pseudo moves
            for(int i = 0; i < n_pseudo_moves; i++){
                Move move(pseudo_moves[i]);
                bool is_white = Piece::getColor(iboard[move.getFrom()]) == Piece::White;
                int king = is_white ? white_king_pos : black_king_pos;
                if (validateMove(move, king, is_white)){
                    addMove(move.getFrom(), move.getTo(), move.getFlags(), const_cast<int*>(move_list.data()), n_moves);
                }
            }
        }

        return n_moves;
    }

    void Manager::generatePseudoLegalMoves(int& n_pseudo_moves, int* pseudo_moves){
        int* iboard = this->board->board.get();

        n_pseudo_moves = 0;
        
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

                if (type == Piece::King){
                    if (is_white){
                        white_king_pos = i;
                    } else {
                        black_king_pos = i;
                    }
                }
                 
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
                        // Check if enpassant is possible, if the enpassant target is the same as the square
                        if (board->enpassantTarget() == n){
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
        if (!board->m_castling_rights){
            dlogln("No castling rights");
            return;
        }

        for(int i = 0; i < 2; i++){
            // Get starting position for the king
            int king = castling_data[i][0]; // 0 black, 1 white
            if (iboard[king] != Piece::getKing(castling_data[i][3])) // King has moved
                continue;

            dlogf("Found valid king at %s\n", square_to_str(king).c_str());
            // Check if rooks have moved / still have castling rights
            for(int j = 0; j < 2; j++){
                checkKingCastling(i, j, king);
            }
        }
    }

    bool Manager::validateMove(Move& move, int king, bool is_white){
        return true;
    }

    /**
     * @brief Check if the king can castle to the given side.
     * @param is_white True if the king is white, false otherwise
     * @param j The index of the castling_data array at position 1 or 2
     * @param king The index of the king
     */
    void Manager::checkKingCastling(bool is_white, int j, int king){
        int *iboard = board->board.get();

        CastlingRights cr(board->m_castling_rights);
        int rook = iboard[castling_data[is_white][j + 1]];
        if (rook != Piece::getRook(castling_data[is_white][3])) // not the correct piece (rook)
            return;

        const int castling_rights[2][2] = {
            {CastlingRights::BLACK_QUEEN, CastlingRights::BLACK_KING},
            {CastlingRights::WHITE_QUEEN, CastlingRights::WHITE_KING}
        };

        // Check if we have correct castling rights
        if(!cr.has(castling_rights[is_white][j]))
            return;
        
        dlogf("Valid rook at %s\n", square_to_str(castling_data[is_white][j + 1]).c_str());
        if (attacks_to[!is_white][king] != 0) // king in check
            return;

        // Check if the squares between the king and the rook are empty and 
        // not attacked by the enemy color
        int pos = king;
        int rook_pos = castling_data[is_white][j + 1];
        int target_king_pos = king + castling_offsets[0][j];

        while(pos != target_king_pos){
            pos += castling_offsets[1][j];
            dlogf("Checking position at %s\n", square_to_str(pos).c_str());
            dbitboard(attacks_to[!is_white][pos]);
            if (iboard[pos] != Piece::Empty || attacks_to[!is_white][pos] != 0) // piece in the way or attacked
                return;
        }

        while(pos != rook_pos){
            if (iboard[pos] != Piece::Empty)
                return;
            pos += castling_offsets[1][j];
        }

        // valid castle
        dlogf("Added possible castle at %s\n", square_to_str(target_king_pos).c_str());
        addMove(king, target_king_pos, castling_flags[j], const_cast<int*>(move_list.data()), n_moves);
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
}