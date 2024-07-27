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

    uint64_t Manager::in_between[64][64] = {};

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

        // Initialize the in_between array
        // Taken from: https://www.chessprogramming.org/Square_Attacked_By
        // This array gives the squares in-between two squares
        // For example, in_between[0][8] gives the squares in-between f6 and c3:
        // . . . . . . . . 8
        // . . . . . . . . 7
        // . . . . . . . . 6
        // . . . . 1 . . . 5
        // . . . 1 . . . . 4
        // . . . . . . . . 3
        // . . . . . . . . 2
        // . . . . . . . . 1
        // a b c d e f g h

        const uint64_t m1   = (-1);
        const uint64_t a2a7 = (0x0001010101010100);
        const uint64_t b2g7 = (0x0040201008040200);
        const uint64_t h1b7 = (0x0002040810204080); /* Thanks Dustin, g2b7 did not work for c1-a3 */
        
        for(int i = 0; i < 64; i++){
            for(int j = 0; j < 64; j++){
                uint64_t btwn, line, rank, file;
                btwn  = (m1 << i) ^ (m1 << j);
                file  =   (j & 7) - (i   & 7);
                rank  =  ((j | 7) -  i) >> 3 ;
                line  =      (   (file  &  7) - 1) & a2a7; /* a2a7 if same file */
                line += 2 * ((   (rank  &  7) - 1) >> 58); /* b1g1 if same rank */
                line += (((rank - file) & 15) - 1) & b2g7; /* b2g7 if same diagonal */
                line += (((rank + file) & 15) - 1) & h1b7; /* h1b7 if same antidiag */
                line *= btwn & -btwn; /* mul acts like shift by smaller square */
                in_between[i][j] = line & btwn;   /* return the bits on that line in-between */
            }
        }
    
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
        bool is_white = Piece::isWhite(iboard[to]);

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
            board->castlingRights().add(castling_rights[1][is_white]);
        } else if (curr_move.isKingCastle()){
            int rook_from = is_white ? 63 : 7; 
            iboard[rook_from] = iboard[to - 1];
            iboard[to - 1] = Piece::Empty;
            board->castlingRights().add(castling_rights[0][is_white]);
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
        bool is_white = Piece::isWhite(iboard[from]);

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
            board->m_castling_rights.remove(Piece::isWhite(iboard[from]) ? CastlingRights::WHITE : CastlingRights::BLACK);
        } else if (Piece::getType(iboard[from]) == Piece::Rook){
            // If the rook is moved from the starting position, remove the castling rights for that side
            if (from == 0 || from == 56){ // queen side rook
                board->m_castling_rights.remove(is_white ? CastlingRights::WHITE_QUEEN : CastlingRights::BLACK_QUEEN);
            } else if (from == 7 || from == 63){ // king side rook
                board->m_castling_rights.remove(is_white ? CastlingRights::WHITE_KING : CastlingRights::BLACK_KING);
            }
        }

        // Move the piece
        iboard[to] = iboard[from];
        iboard[from] = Piece::Empty;

        // Update the bitboard for the piece
        board->updateBitboard(is_white, Piece::getType(iboard[to]) - 1, from, to);
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
        bool is_white = Piece::isWhite(iboard[from]);

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
        board->castlingRights().remove(is_white ? CastlingRights::WHITE : CastlingRights::BLACK);

        // Move the rook (king will be handled in `handleMove()`)
        iboard[rook_to] = iboard[rook_from]; 
        iboard[rook_from] = Piece::Empty;

        // Update the bitboard for the rook
        board->updateBitboard(is_white, Piece::getType(iboard[rook_to]) - 1, rook_from, rook_to);
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
            offset = Piece::isWhite(iboard[from]) ? 8 : -8;
            dlogf("En passant capture\n");
        }

        prev_captured_piece = captured_piece;
        captured_piece = iboard[to + offset];
        iboard[to + offset] = Piece::Empty;

        // Update the bitboard for the captured piece
        board->bitboards(Piece::isWhite(captured_piece))[Piece::getType(captured_piece) - 1] ^= 1ULL << (to + offset);
    }

    /**
     * @brief Generates all possible moves for the current board state
     * @return The number of moves generated
     */
    int Manager::generateMoves(bool validate)
    {
        this->n_moves = 0;
        int* iboard = this->board->getBoard();

        int n_pseudo_moves = 0;
        std::unique_ptr<int[]> pseudo_moves(new int[256]);
        generatePseudoLegalMoves(n_pseudo_moves, pseudo_moves.get());

    #if DEBUG_DETAILS
        // print bitboards
        // for(int i = 0; i < 2; i++){
        //     uint64_t *bb = board->bitboards(i);
        //     uint64_t bb_combined = 0;

        //     for(int i = 0; i < 6; i++){
        //         bb_combined |= bb[i];
        //     }

        //     dlogf("Bitboard for %s\n", i == 0 ? "black" : "white");
        //     dbitboard(bb_combined);
        // }
    #endif

        // TODO: Implement pinned pieces
        // Source: https://www.chessprogramming.org/Checks_and_Pinned_Pieces_(Bitboards)
        bool is_white = board->getSide() == Piece::White;
        int king = is_white ? white_king_pos : black_king_pos;

        uint64_t pinned = 0;
        uint64_t occupied = board->occupied();
        uint64_t blockers = board->occupied(is_white);
        uint64_t pinner = xRayRookAttacks(occupied, blockers, king) & board->oppRooksQueens(is_white);

        dlogf("Pinned pieces for %s\n", square_to_str(king).c_str());
        dbitboard(occupied);
        dbitboard(blockers);
        dbitboard(pinner);
        dbitboard(board->oppRooksQueens(is_white));
        dbitboard(board->oppBishopsQueens(is_white));
        dlogf("X-ray attacks for %s\n", square_to_str(king).c_str());
        dbitboard(xRayRookAttacks(occupied, blockers, king));
        dbitboard(xRayBishopAttacks(occupied, blockers, king));

        while(pinner){
            int sq = bitScanForward(pinner);
            pinned |= in_between[sq][king] & blockers;
            pinner &= pinner - 1;
        }
        pinner = xRayBishopAttacks(occupied, blockers, king) & board->oppBishopsQueens(is_white);
        while(pinner){
            int sq = bitScanForward(pinner);
            pinned |= in_between[sq][king] & blockers;
            pinner &= pinner - 1;
        }

        dlogf("Pinned pieces for %s\n", square_to_str(king).c_str());
        dbitboard(pinned);


        // validate pseudo moves
        for(int i = 0; i < n_pseudo_moves; i++){
            Move move(pseudo_moves[i]);
            int color = Piece::getColor(iboard[move.getFrom()]);

            // If the piece is not the same color as the side to move, skip the move
            if (color != board->getSide())
                continue;
            
            bool is_white = color == Piece::White;
            int king = is_white ? white_king_pos : black_king_pos;
            if (validateMove(move, king, is_white, pinned)){
                addMove(move.getFrom(), move.getTo(), move.getFlags(), const_cast<int*>(move_list.data()), n_moves);
            }
        }
        

        return n_moves;
    }

    /**
     * @brief Generates the attacks for a slider piece
     * @param type The type of the piece (Piece::Rook, Piece::Bishop, Piece::Queen)
     * @param occupied The bitboard of the occupied squares
     * @param square The square of the piece
     */
    uint64_t sliderTypeAttacks(int type, uint64_t occupied, int square){
        uint64_t attacks = 0;
        for(int j = 0; j < Board::n_piece_rays[type]; j++){
            for(int n = square;;){
                // mailbox64 has indexes for the 64 valid squares in mailbox.
                // If, by moving the piece, we go outside of the valid squares (n == -1),
                // we break the loop. Else, the n has the index of the next square.
                n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][j]];
                if (n == -1){// outside of the board
                    break;
                }
                
                // Attack = possible move 
                attacks |= 1ULL << n;

                // If the square is not empty
                if (occupied & (1ULL << n)){
                    break;
                }
            }
        }
        return attacks;
    }

    /**
     * @brief Get the rook attacks for a given square, attacks are both squares attacked and pieces
     */
    uint64_t Manager::rookAttacks(uint64_t occupied, int rook){
        return sliderTypeAttacks(Piece::Rook - 1, occupied, rook);
    }

    /**
     * @brief Get the bishop attacks for a given square
     */
    uint64_t Manager::bishopAttacks(uint64_t occupied, int bishop){
        return sliderTypeAttacks(Piece::Bishop - 1, occupied, bishop);
    }

    /**
     * @brief Get the x-ray attacks for a rook
     * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
     */
    uint64_t Manager::xRayRookAttacks(uint64_t occupied, uint64_t blockers, int rooksq){
        uint64_t attacks = rookAttacks(occupied, rooksq); // get the valid rook attacks (all pieces involved)
        blockers &= attacks; // take only the blockers that are in the attacks (attacked blockers)
        return attacks ^ rookAttacks(occupied ^ blockers, rooksq); // get the x-ray attacks (attacks through the blockers)
    }
    
    /**
     * @brief Get the x-ray attacks for a bishop
     * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
     */
    uint64_t Manager::xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int bishopsq){
        uint64_t attacks = bishopAttacks(occupied, bishopsq);
        blockers &= attacks;
        return attacks ^ bishopAttacks(occupied ^ blockers, bishopsq);
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

    bool Manager::validateMove(Move& move, int king, bool is_white, uint64_t pinned){
        int to = move.getTo();
        int from = move.getFrom();
        int king_attack = attacks_to[!is_white][king];

        if (from != king){
            // Check if the king is not in check
            if (king_attack == 0){
                // Check if the piece is not pinned
                uint64_t bitmove = 1ULL << from;
                return (pinned & bitmove) == 0;
            }
            
            // If there is more than one piece attacking the king, the move is invalid
            if (king_attack & (king_attack - 1)){
                // More than one piece is attacking the king
                return false;
            }

            // Get the position of the attacking piece (attacks_to = 1 << pos)
            int attacking_pos = bitScanForward(king_attack);

            // Check if the the given move can block the check or capture the attacking piece
            if (move.isCapture()){
                // Check if the move is not a capture of the attacking piece
                if (attacking_pos != to){
                    return false;
                }
            } else {
                // Check if the move can block the check, we will need to the bitboards, since the attacks
                // are also bitboards
                // int bitmove = 1 << to;

                // TODO: Implement blocking check
                uint64_t bitmove = 1ULL << to;
                if ((in_between[attacking_pos][king] & bitmove) == 0){
                    return false;
                }
            }

        } else {
            // Check if the king can't move to the given square
            if (attacks_to[!is_white][to] != 0){
                return false;
            }
        }

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

        int rook = iboard[castling_data[is_white][j + 1]];
        if (rook != Piece::getRook(castling_data[is_white][3])) // not the correct piece (rook)
            return;

        const int castling_rights[2][2] = {
            {CastlingRights::BLACK_QUEEN, CastlingRights::BLACK_KING},
            {CastlingRights::WHITE_QUEEN, CastlingRights::WHITE_KING}
        };

        // Check if we have correct castling rights
        if(!board->castlingRights().has(castling_rights[is_white][j]))
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