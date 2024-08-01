#include "cengine/manager_impl.h"


namespace chess{
    // Constants

    constexpr int WHITE_KING_START = 60, BLACK_KING_START = 4;

    uint64_t ManagerImpl::attacks_to[2][64] = {};
    uint64_t ManagerImpl::attacks_from[2][64] = {};

    const int ManagerImpl::castling_data[2][4] = {
        {BLACK_KING_START,  0,  7, Piece::Black}, // Black (index 0 -> is_white = false)
        {WHITE_KING_START, 56, 63, Piece::White} // White
    };

    const int ManagerImpl::castling_offsets[2][2] = {
        { -2, 2}, // target relative position (king + offset -> target position)
        { -1, 1}, // position offset (pos += offset while pos != king)
    };

    const int ManagerImpl::castling_flags[2] = {
        Move::FLAG_QUEEN_CASTLE, Move::FLAG_KING_CASTLE
    };

    uint64_t ManagerImpl::in_between[64][64] = {};
    uint64_t ManagerImpl::pawnAttacks[2][64] = {};

    // Helper functions

    /**
     * @brief Generates the attacks for a piece
     * @param type The type of the piece (Piece::Rook, Piece::Bishop, Piece::Queen)
     * @param occupied The bitboard of the occupied squares
     * @param square The square of the piece
     */
    uint64_t mailboxAttacks(int type, uint64_t occupied, int square, bool is_sliding = true)
    {
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

                if (!is_sliding){
                    break;
                }
            }
        }
        return attacks;
    }

    // C'tor

    ManagerImpl::ManagerImpl(Board* board)
    {
        this->state = GameState::Normal;
        this->captured_piece = Piece::Empty;
        this->n_moves = 0;
        this->move_list = std::vector<int>(256, 0);

        // load data from the board
        this->board = board;

        if (board == nullptr)
            return;
        init();
        generateMoves();
        pushHistory();
    }

    ManagerImpl& ManagerImpl::operator=(ManagerImpl&& other)
    {
        this->board = other.board;
        this->n_moves = other.n_moves;
        this->move_list = std::move(other.move_list);
        this->captured_piece = other.captured_piece;
        this->curr_move = other.curr_move;
        this->prev_move = other.prev_move;
        this->black_king_pos = other.black_king_pos;
        this->white_king_pos = other.white_king_pos;
        this->history = other.history;
        this->state = other.state;
        return *this;
    }

    void ManagerImpl::init()
    {
        white_king_pos = bitScanForward(board->bitboards(true)[Piece::King - 1]);
        black_king_pos = bitScanForward(board->bitboards(false)[Piece::King - 1]);

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

        // Init piece attacks
        for(int i = 0; i < 64; i++){
            // Pawn attacks
            for(int j = 0; j < 2; j++){
                for(int k = 0; k < 2; k++){
                    int n = Board::mailbox[Board::mailbox64[i] + Board::pawn_attack_offsets[j][k]];
                    if(n != -1){
                        pawnAttacks[j][i] |= 1ULL << n;
                    }
                }
            }
        }
    }

    /**
     * @brief Reloads the manager with a new board state, resetting all variables and generating moves
     */
    void ManagerImpl::reload()
    {
        this->state = GameState::Normal;
        this->captured_piece = Piece::Empty;
        this->curr_move = Move();
        this->n_moves = 0;
        this->move_list = std::vector<int>(256, 0);
        this->history.clear();
        this->black_king_pos = bitScanForward(board->bitboards(false)[Piece::King - 1]);
        this->white_king_pos = bitScanForward(board->bitboards(true)[Piece::King - 1]);
        generateMoves();
        pushHistory();
    }

    /**
     * @brief Makes a move, updating the board, the side to move and the move history
     * @attention User should call `generateMoves()` after calling this function
     */
    void ManagerImpl::make(Move& move)
    {
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
        pushHistory();
    }

    /**
     * @brief Unmakes current move, restoring the board to the previous state, 
     * may be called only once after `make()`
     * @attention User should call `generateMoves()` after calling this function
     */
    void ManagerImpl::unmake()
    {
        if(this->history.size() == 1)
            return;

        // Pop current move from history & get the previous one
        this->history.pop_back();
        auto history = this->history.back();
        
        int* iboard = board->getBoard();
        int from = curr_move.getFrom(), to = curr_move.getTo(), captured_pos = to;
        bool is_white = history.side_to_move == Piece::White;
        const int colors[2] = { Piece::Black, Piece::White };

        if (curr_move.isCapture()){
            int offset = 0;
            if(curr_move.isEnPassant()){
                offset = is_white ? 8 : -8;
            }
            captured_pos += offset;
            // Restore the captured piece in bitboards
            board->bitboards(!is_white)[Piece::getType(captured_piece) - 1] |= 1ULL << captured_pos;
        } else if (curr_move.isCastle()){
            const int rooks_to[2][2] = {
                {0, 56}, // queen castle
                {7, 63} // king castle
            };
            const int rooks_from[2][2] = {
                {3, 59}, // queen castle
                {5, 61} // king castle
            };
            // Get rook starting position
            bool is_king_castle = curr_move.isKingCastle();
            int rook_from = rooks_from[is_king_castle][is_white];
            int rook_to = rooks_to[is_king_castle][is_white];
            iboard[rook_to] = iboard[rook_from]; // move the rook back
            iboard[rook_from] = Piece::Empty; // empty the rook's previous position
            board->updateBitboard(is_white, Piece::Rook - 1, rook_from, rook_to); // update the bitboard
        }

        // Restore the promotion piece
        if (curr_move.isPromotion()){
            board->bitboards(is_white)[Piece::getType(iboard[to]) - 1] ^= 1ULL << to; // remove the promoted piece
            iboard[to] = Piece::Pawn | colors[is_white]; // restore the pawn
            board->bitboards(is_white)[Piece::Pawn - 1] |= 1ULL << to; // add the pawn to the bitboard (not moved yet)
        }

        // Restore the king position
        if (Piece::getType(iboard[to]) == Piece::King){
            int *king_pos = is_white ? &white_king_pos : &black_king_pos;
            *king_pos = from;
        }

        // Restore the moved piece
        iboard[from] = iboard[to];
        iboard[to] = Piece::Empty;
        board->updateBitboard(is_white, Piece::getType(iboard[from]) - 1, to, from);
        // Restore the captured piece
        iboard[captured_pos] = captured_piece;

        // restore other variables
        this->board->getSide() = history.side_to_move;
        this->board->halfmoveClock() = history.halfmove_clock;
        this->board->enpassantTarget() = history.enpassant_target;
        this->board->castlingRights() = history.castling_rights;
        this->board->fullmoveCounter() = history.fullmove_counter;
        this->captured_piece = history.captured_piece;
        this->curr_move = history.move;
    }

    /**
     * @brief Moves the piece to given position, it handles castling, promotion, double pawn move
     */
    void ManagerImpl::handleMove(Move& move)
    {
        // Reset enpassant target every move
        board->enpassantTarget() = 0;
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
            board->enpassantTarget() = to + direction;
        }

        // If that's a promotion, update the piece
        if(move.isPromotion()){
            const int promotion_piece[4] = { Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen };
            iboard[from] = promotion_piece[move.getPromotionPiece()] | (board->getSide()); // set the piece with the correct color
            board->bitboards(is_white)[Piece::Pawn - 1] ^= 1ULL << from; // remove the pawn
            board->bitboards(is_white)[Piece::getType(iboard[from]) - 1] |= 1ULL << from; // add the promoted piece (not moved yet)
        }

        // Update castling rights & king position
        if (Piece::getType(iboard[from]) == Piece::King){
            int *king_pos = is_white ? &white_king_pos : &black_king_pos;
            *king_pos = to;
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
        dbitboard(board->bitboards(is_white)[Piece::getType(iboard[to]) - 1]);
    }

    /**
     * @brief Moves the rook to the correct position after a castling move
     * @param is_king_castle True if it's a king castle, false otherwise
     * @param from The starting position of the king
     * @param to The target position of the king
     */
    void ManagerImpl::handleCastlingMove(bool is_king_castle, int from, int to)
    {
        int* iboard = board->board.get();
        bool is_white = Piece::isWhite(iboard[from]);

        const int castling_pos[2][2] = {
            {0, 56}, // queen castle 
            {7, 63} // king castle
        };
        const int rooks_to[2][2] = {
            {3, 59}, // queen castle
            {5, 61} // king castle
        };
        const int rights[2] = { CastlingRights::BLACK, CastlingRights::WHITE };

        int rook_from = castling_pos[is_king_castle][is_white],
            rook_to = rooks_to[is_king_castle][is_white];

        // Update castling rights
        board->castlingRights().remove(rights[is_white]);

        // Move the rook (king will be handled in `handleMove()`)
        iboard[rook_to] = iboard[rook_from]; 
        iboard[rook_from] = Piece::Empty;

        // Update the bitboard for the rook
        board->updateBitboard(is_white, Piece::Rook - 1, rook_from, rook_to);
    }

    /**
     * @brief If the move is a capture, it correctly updates the board
     * @attention Should be called before `handleMove()` as it won't update correctly `captured_piece`
     */
    void ManagerImpl::handleCapture(Move& move)
    {
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

        captured_piece = iboard[to + offset];
        iboard[to + offset] = Piece::Empty;

        // Update the bitboard for the captured piece
        board->bitboards(Piece::isWhite(captured_piece))[Piece::getType(captured_piece) - 1] ^= 1ULL << (to + offset);
    }

    /**
     * @brief Generates all possible moves for the current board state
     * @return The number of moves generated
     */
    int ManagerImpl::generateMoves()
    {
        if (state != GameState::Normal)
            return 0;

        dtimer_t timer;
        start_timer(timer);

        this->n_moves = 0;
        int n_pseudo_moves = 0;
        int* iboard = this->board->getBoard();
        bool is_white = board->getSide() == Piece::White;
        const int kings[2] = {black_king_pos, white_king_pos};

        std::unique_ptr<int[]> pseudo_moves(new int[256]);
        
        for(int i = 0; i < 64; i++){
            // reset the attacks_from array & attacks_to bitboards
            attacks_from[0][i] = 0; 
            attacks_from[1][i] = 0; 
            attacks_to[0][i] = 0;
            attacks_to[1][i] = 0;
        }

        // Generate pseudo-legal moves
        uint64_t occupied = board->occupied();
        uint64_t blockers = board->occupied(is_white);
        int king = kings[is_white];

        // First generate for the opposite side with the king removed (for xRay attacks)
        generatePseudoMoves(!is_white, occupied ^ (1ULL << king), blockers ^ (1ULL << king), nullptr);
        n_pseudo_moves = generatePseudoMoves(is_white, occupied, board->occupied(!is_white), pseudo_moves.get());   

        dlogln("Checking castle rights");

        // Check for castling
        if (board->castlingRights()){
            // Get starting position for the king
            int king = castling_data[is_white][0]; // 0 black, 1 white
            if (iboard[king] == Piece::getKing(castling_data[is_white][3])){
                dlogf("Found valid king at %s\n", square_to_str(king).c_str());
                // Check if rooks have moved / still have castling rights
                for(int j = 0; j < 2; j++){
                    checkKingCastling(is_white, j, king);
                }
            }
        }

        // TODO: Implement pinned pieces
        // Source: https://www.chessprogramming.org/Checks_and_Pinned_Pieces_(Bitboards)
        uint64_t pinned = 0;
        uint64_t pinners = 0;
        uint64_t in_between_bb = 0;
        uint64_t pinner = xRayRookAttacks(occupied, blockers, king) & board->oppRooksQueens(is_white);
        pinners |= pinner;

        while(pinner){
            int sq = bitScanForward(pinner);
            pinned |= in_between[sq][king] & blockers;
            in_between_bb |= in_between[sq][king];
            pinner &= pinner - 1;
        }
        pinner = xRayBishopAttacks(occupied, blockers, king) & board->oppBishopsQueens(is_white);
        pinners |= pinner;

        while(pinner){
            int sq = bitScanForward(pinner);
            pinned |= in_between[sq][king] & blockers;
            in_between_bb |= in_between[sq][king];
            pinner &= pinner - 1;
        }

        // validate pseudo moves
        for(int i = 0; i < n_pseudo_moves; i++){
            Move move(pseudo_moves[i]);
            // If the piece is not the same color as the side to move, skip the move
            if (Piece::getColor(iboard[move.getFrom()]) != board->getSide())
                continue;
            
            if (validateMove(move, king, is_white, pinned, pinners, in_between_bb)){
                addMove(move.getFrom(), move.getTo(), move.getFlags(), const_cast<int*>(move_list.data()), n_moves);
            }
        }

        // Evaluate the game state
        state = evalState();
        
        end_timer(timer, "Move generation");
        return n_moves;
    }

    /**
     * @brief Generates pseudo-legal moves for a given side
     * @param is_white True if the side is white, false otherwise
     * @param occupied The bitboard of the occupied squares
     * @param enemy_pieces The bitboard of the enemy pieces
     * @param move_list The list of moves to append to
     * @return The number of pseudo-legal moves generated, if the current side is not the same as the side to generate moves for
     * it will only save moves to the attacks table (`attacks_to`, `attacks_from`)
     */
    int ManagerImpl::generatePseudoMoves(bool is_white, uint64_t occupied, uint64_t enemy_pieces, int* move_list)
    {
        // Pawn moves
        uint64_t pawns = board->bitboards(is_white)[Piece::Pawn - 1];
        bool current_side = (board->getSide() == Piece::White) == is_white;
        int n_pseudo_moves = 0;

        // Special ranks for double pawn moves & promotions
        const int special_ranks[2] = {
            1, 6, 
        };

        // Handle pawn moves
        while(pawns){
            int from = bitScanForward(pawns);
            int rank = from >> 3;
            uint64_t attacks = pawnAttacks[is_white][from];

            // Check for captures
            while(attacks){
                int n = bitScanForward(attacks);
                addAttack(from, n, is_white);
                if (current_side && (((1ULL << n) & enemy_pieces) != 0)){
                    if (rank == special_ranks[!is_white]){
                        // Check for promotions
                        addMove(from, n, Move::FLAG_ROOK_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
                        addMove(from, n, Move::FLAG_QUEEN_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
                        addMove(from, n, Move::FLAG_BISHOP_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
                        addMove(from, n, Move::FLAG_KNIGHT_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
                    } else {
                        // Just a normal capture
                        addMove(from, n, Move::FLAG_CAPTURE, move_list, n_pseudo_moves);
                    }
                }                
                attacks &= (attacks - 1);
            }

            if (!current_side){
                pawns &= (pawns - 1);
                continue;
            }

            // Check for enpassant
            if((board->enpassantTarget() != 0) && (1ULL << board->enpassantTarget()) & pawnAttacks[is_white][from]){
                addMove(from, board->enpassantTarget(), Move::FLAG_ENPASSANT_CAPTURE, move_list, n_pseudo_moves);
            }

            // Check for pawn moves
            int n = Board::mailbox[Board::mailbox64[from] + Board::pawn_move_offsets[is_white][0]];
            if (n != -1 && (occupied & (1ULL << n)) == 0){
                
                // Check if that's a promotion move
                if (rank == special_ranks[!is_white]){
                    addMove(from, n, Move::FLAG_ROOK_PROMOTION, move_list, n_pseudo_moves);
                    addMove(from, n, Move::FLAG_QUEEN_PROMOTION, move_list, n_pseudo_moves);
                    addMove(from, n, Move::FLAG_BISHOP_PROMOTION, move_list, n_pseudo_moves);
                    addMove(from, n, Move::FLAG_KNIGHT_PROMOTION, move_list, n_pseudo_moves);
                } else {
                    addMove(from, n, Move::FLAG_NONE, move_list, n_pseudo_moves);
                }                

                if (rank == special_ranks[is_white]){
                    // Check for double pawn moves
                    n = Board::mailbox[Board::mailbox64[from] + Board::pawn_move_offsets[is_white][1]];
                    if ((occupied & (1ULL << n)) == 0){
                        addMove(from, n, Move::FLAG_DOUBLE_PAWN, move_list, n_pseudo_moves);
                    }
                }
            }

            pawns &= (pawns - 1);
        }

        for(int type = 1; type < 6; type++){
            uint64_t pieces = board->bitboards(is_white)[type];
            // Current side generation (adding to the move list)
            if(current_side){
                while(pieces){
                    int from = bitScanForward(pieces);
                    for(int rays = 0; rays < Board::n_piece_rays[type]; rays++){
                        for(int n = from;;){
                            n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][rays]];
                            if (n == -1){
                                break;
                            }
                            uint64_t bitmove = 1ULL << n;
                            addAttack(from, n, is_white);
                            if (occupied & bitmove){
                                if (enemy_pieces & bitmove){
                                    addMove(from, n, Move::FLAG_CAPTURE, move_list, n_pseudo_moves);
                                }
                                break;
                            }

                            addMove(from, n, Move::FLAG_NONE, move_list, n_pseudo_moves);
                            if (!Board::is_piece_sliding[type]){
                                break;
                            }
                        }
                    }
                    pieces &= (pieces - 1);
                }
            } else {
                // Enemy pieces generation (only adding to the attacks table)
                while(pieces){
                    int from = bitScanForward(pieces);
                    for(int rays = 0; rays < Board::n_piece_rays[type]; rays++){
                        for(int n = from;;){
                            n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][rays]];
                            if (n == -1){
                                break;
                            }
                            addAttack(from, n, is_white);
                            if (occupied & (1ULL << n)){
                                break;
                            }
                            if (!Board::is_piece_sliding[type]){
                                break;
                            }
                        }
                    }
                    pieces &= (pieces - 1);
                }                
            }
        }

        return n_pseudo_moves;
    }

    /**
     * @brief Get the rook attacks for a given square, attacks are both squares attacked and pieces
     */
    uint64_t ManagerImpl::rookAttacks(uint64_t occupied, int rook)
    {
        return mailboxAttacks(Piece::Rook - 1, occupied, rook);
    }

    /**
     * @brief Get the bishop attacks for a given square
     */
    uint64_t ManagerImpl::bishopAttacks(uint64_t occupied, int bishop)
    {
        return mailboxAttacks(Piece::Bishop - 1, occupied, bishop);
    }

    /**
     * @brief Get the x-ray attacks for a rook
     * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
     */
    uint64_t ManagerImpl::xRayRookAttacks(uint64_t occupied, uint64_t blockers, int rooksq)
    {
        uint64_t attacks = rookAttacks(occupied, rooksq); // get the valid rook attacks (all pieces involved)
        blockers &= attacks; // take only the blockers that are in the attacks (attacked blockers)
        return attacks ^ rookAttacks(occupied ^ blockers, rooksq); // get the x-ray attacks (attacks through the blockers)
    }
    
    /**
     * @brief Get the x-ray attacks for a bishop
     * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
     */
    uint64_t ManagerImpl::xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int bishopsq)
    {
        uint64_t attacks = bishopAttacks(occupied, bishopsq);
        blockers &= attacks;
        return attacks ^ bishopAttacks(occupied ^ blockers, bishopsq);
    }

    bool ManagerImpl::validateMove(Move& move, int king, bool is_white, uint64_t pinned, uint64_t pinners, uint64_t in_between_bb)
    {
        int to = move.getTo();
        int from = move.getFrom();
        uint64_t king_attack = attacks_to[!is_white][king];
        const int enpassant_dir[2] = {-8, 8};

        if (from != king){
            // Check if the king is not in check
            if (king_attack == 0){
                // Check if the piece is not pinned
                uint64_t bitmove = 1ULL << from;

                // Check if the move is enpassant (special pawn case)
                if (move.isEnPassant()){
                    // Check with x-ray attacks, without the pawn target
                    uint64_t occupied = board->occupied() ^ (1ULL << (to + enpassant_dir[is_white]));
                    uint64_t blockers = board->occupied(is_white);
                    uint64_t pinner = xRayRookAttacks(occupied, blockers, king) & board->oppRooksQueens(is_white);
                    while(pinner){
                        int sq = bitScanForward(pinner);
                        pinned |= in_between[sq][king] & blockers;
                        pinner &= pinner - 1;
                    }
                }

                if (pinned & bitmove){
                    // The piece is pinned, so it can only move along the pin line or capture the pinning piece
                    return ((in_between_bb | pinners) & (1ULL << to)) != 0;
                }               

                return true;
            }
            
            // If there is more than one piece attacking the king, the move is invalid
            if (king_attack & (king_attack - 1)){
                // More than one piece is attacking the king
                return false;
            }

            int attacker = bitScanForward(king_attack);
            const int offsets[2] = {0, enpassant_dir[is_white]};
            int offset = offsets[move.isEnPassant()];

            // If the piece is pinned it cannot capture the checking piece
            if (pinned & (1ULL << from)){
                return false;
            }

            // Check if the move can block the check or capture the attacking piece
            return (in_between[attacker][king] | king_attack) & (1ULL << (to + offset));
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
    void ManagerImpl::checkKingCastling(bool is_white, int j, int king){
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
     * @brief Evaluates the current game state, should be called after `generateMoves()`
     */
    ManagerImpl::GameState ManagerImpl::evalState(){

        // fifty-move rule
        if (board->halfmoveClock() >= 100){
            return GameState::Draw;
        }

        // If there are moves available
        if (n_moves != 0){
            // TODO:
            // Threefold repetition
            return GameState::Normal;
        }

        // There are no moves available, so check if the king is in check
        const int kings[2] = {black_king_pos, white_king_pos};
        bool is_white = board->getSide() == Piece::White;
        uint64_t king_attack = attacks_to[!is_white][kings[is_white]];
        
        if (king_attack == 0){
            // Stalemate
            return GameState::Draw;
        }
        return GameState::Checkmate;
    }

    /**
     * @brief Pushes the current move to the history stack
     */
    void ManagerImpl::pushHistory(){
        CHistory new_history;
        new_history.move = curr_move.move();
        new_history.side_to_move = board->getSide();
        new_history.captured_piece = captured_piece;
        new_history.enpassant_target = board->enpassantTarget();
        new_history.halfmove_clock = board->halfmoveClock();
        new_history.fullmove_counter = board->fullmoveCounter();
        new_history.castling_rights = board->castlingRights().rights();
        new_history.game_state = this->state;

        history.push_back(new_history);
    }

    /**
     * @brief Adds a move to the move list
     */
    void ManagerImpl::addMove(int from, int to, int flags, int* move_list, int& n_moves){
        move_list[n_moves++] = int(Move(from, to, flags));
    }

    /**
     * @brief Updates `attacks_to` and `attacks_from` bitboards
     * @param from The square from which the attack is coming
     * @param to The square being attacked
     * @param is_white True if the attacking piece is white, false otherwise
     */
    void ManagerImpl::addAttack(int from, int to, bool is_white){
        // basically, attacks_to[is_white][x] is usefull if you want to check how many pieces are attacking 
        // given square x, and attacks_from[is_white][x] if you want to check how many squares are being attacked
        // by the piece in square x. The `is_white` variable is used to differentiate between white and black pieces
        attacks_to[is_white][to] |= 1UL << from; 
        attacks_from[is_white][from] |= 1UL << to;
    }
}