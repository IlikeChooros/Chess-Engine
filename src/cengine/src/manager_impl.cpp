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

    // C'tor

    ManagerImpl::ManagerImpl(Board* board)
    {
        this->state = GameState::Normal;
        this->captured_piece = Piece::Empty;
        this->n_moves = 0;
        this->move_list = std::vector<uint32_t>(256, 0);
        this->board = board;
        this->curr_move = Move();

        if (board == nullptr)
            return;

        pushHistory();
    }

    ManagerImpl& ManagerImpl::operator=(ManagerImpl&& other)
    {
        this->board = other.board;
        this->n_moves = other.n_moves;
        this->move_list = std::move(other.move_list);
        this->captured_piece = other.captured_piece;
        this->curr_move = other.curr_move;
        this->history = other.history;
        this->state = other.state;
        return *this;
    }

    /**
     * @brief Initializes the board bitboards, should be called once before generating moves
     */
    void ManagerImpl::init()
    {
        init_board(board);
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
        this->move_list = std::vector<uint32_t>(256, 0);
        this->history.clear();
        (void)generateMoves();
        pushHistory();
    }

    /**
     * @brief Makes a move, updating the board, the side to move and the move history
     * @attention User should call `generateMoves()` after calling this function
     */
    void ManagerImpl::make(Move& move)
    {
        ::make(move, board, &history);
        curr_move = move;


        // if(this->board->m_side == Piece::Black){
        //     this->board->fullmoveCounter()++;
        // }
        // curr_move = move;

        // // Update halfmove clock, if the move is a pawn move or a capture, reset the clock
        // // (`handleCapture()` will update the clock if the move is a capture)
        // if (Piece::getType((*board)[curr_move.getFrom()]) == Piece::Pawn){
        //     this->board->halfmoveClock() = 0;
        // } else {
        //     this->board->halfmoveClock()++;
        // }

        // handleCapture(move);
        // handleMove(move);
        // this->board->m_side ^= Piece::colorMask;
        // pushHistory();
    }

    /**
     * @brief Unmakes current move, restoring the board to the previous state, 
     * may be called only once after `make()`
     * @attention User should call `generateMoves()` after calling this function
     */
    void ManagerImpl::unmake()
    {
        ::unmake(curr_move, board, &history);
    }

    /**
     * @brief Pushes the current move to the history stack
     */
    void ManagerImpl::pushHistory(){
        history.push(board, curr_move);
    }

    /**
     * @brief Generate all legal moves for the current board state
     */
    int ManagerImpl::generateMoves(){
        MoveList ml;
        void(::gen_legal_moves(&ml, board));
        move_list = std::vector<uint32_t>(ml.begin(), ml.end());
        n_moves = ml.size();
        return n_moves;
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
        bool is_white = board->getSide() == Piece::White;
        uint64_t king_attack = attacks_to[!is_white][bitScanForward(board->bitboards(is_white)[Piece::King - 1])];
        
        if (king_attack == 0){
            // Stalemate
            return GameState::Draw;
        }
        return GameState::Checkmate;
    }

    // /**
    //  * @brief Moves the piece to given position, it handles castling, promotion, double pawn move
    //  */
    // void ManagerImpl::handleMove(Move& move)
    // {
    //     // Reset enpassant target every move
    //     board->enpassantTarget() = 0;
    //     int* iboard = board->board.get();        
    //     int from = move.getFrom(), to = move.getTo();
    //     bool is_white = Piece::isWhite(iboard[from]);

    //     // If that's a castle, move the corresponding rooks
    //     if (move.isCastle()){
    //         handleCastlingMove(move.isKingCastle(), from, to);
    //     }

    //     // If that's a double pawn move, set the enpassant target
    //     if (move.isDoubleMove()){
    //         int direction = is_white ? 8 : -8;
    //         board->enpassantTarget() = to + direction;
    //     }

    //     // If that's a promotion, update the piece
    //     if(move.isPromotion()){
    //         const int promotion_piece[4] = { Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen };
    //         iboard[from] = promotion_piece[move.getPromotionPiece()] | (board->getSide()); // set the piece with the correct color
    //         board->bitboards(is_white)[Piece::Pawn - 1] ^= 1ULL << from; // remove the pawn
    //         board->bitboards(is_white)[Piece::getType(iboard[from]) - 1] |= 1ULL << from; // add the promoted piece (not moved yet)
    //     }

    //     // Update castling rights & king position
    //     if (Piece::getType(iboard[from]) == Piece::King){
    //         board->m_castling_rights.remove(Piece::isWhite(iboard[from]) ? CastlingRights::WHITE : CastlingRights::BLACK);
    //     } else if (Piece::getType(iboard[from]) == Piece::Rook){
    //         // If the rook is moved from the starting position, remove the castling rights for that side
    //         if (from == 0 || from == 56){ // queen side rook
    //             board->m_castling_rights.remove(is_white ? CastlingRights::WHITE_QUEEN : CastlingRights::BLACK_QUEEN);
    //         } else if (from == 7 || from == 63){ // king side rook
    //             board->m_castling_rights.remove(is_white ? CastlingRights::WHITE_KING : CastlingRights::BLACK_KING);
    //         }
    //     }

    //     // Move the piece
    //     iboard[to] = iboard[from];
    //     iboard[from] = Piece::Empty;

    //     // Update the bitboard for the piece
    //     board->updateBitboard(is_white, Piece::getType(iboard[to]) - 1, from, to);
    //     dbitboard(board->bitboards(is_white)[Piece::getType(iboard[to]) - 1]);
    // }

    // /**
    //  * @brief Moves the rook to the correct position after a castling move
    //  * @param is_king_castle True if it's a king castle, false otherwise
    //  * @param from The starting position of the king
    //  * @param to The target position of the king
    //  */
    // void ManagerImpl::handleCastlingMove(bool is_king_castle, int from, int to)
    // {
    //     int* iboard = board->board.get();
    //     bool is_white = Piece::isWhite(iboard[from]);

    //     const int castling_pos[2][2] = {
    //         {0, 56}, // queen castle 
    //         {7, 63} // king castle
    //     };
    //     const int rooks_to[2][2] = {
    //         {3, 59}, // queen castle
    //         {5, 61} // king castle
    //     };
    //     const int rights[2] = { CastlingRights::BLACK, CastlingRights::WHITE };

    //     int rook_from = castling_pos[is_king_castle][is_white],
    //         rook_to = rooks_to[is_king_castle][is_white];

    //     // Update castling rights
    //     board->castlingRights().remove(rights[is_white]);

    //     // Move the rook (king will be handled in `handleMove()`)
    //     iboard[rook_to] = iboard[rook_from]; 
    //     iboard[rook_from] = Piece::Empty;

    //     // Update the bitboard for the rook
    //     board->updateBitboard(is_white, Piece::Rook - 1, rook_from, rook_to);
    // }

    // /**
    //  * @brief If the move is a capture, it correctly updates the board
    //  * @attention Should be called before `handleMove()` as it won't update correctly `captured_piece`
    //  */
    // void ManagerImpl::handleCapture(Move& move)
    // {
    //     if (!move.isCapture()){
    //         captured_piece = Piece::Empty;
    //         return;
    //     }

    //     board->m_halfmove_clock = 0;
    //     int* iboard = this->board->board.get();
    //     int to = move.getTo();
    //     int from = move.getFrom();
    //     int offset = 0;

    //     if(move.isEnPassant()){
    //         offset = Piece::isWhite(iboard[from]) ? 8 : -8;
    //         dlogf("En passant capture\n");
    //     }

    //     captured_piece = iboard[to + offset];
    //     iboard[to + offset] = Piece::Empty;

    //     // Update the bitboard for the captured piece
    //     board->bitboards(Piece::isWhite(captured_piece))[Piece::getType(captured_piece) - 1] ^= 1ULL << (to + offset);
    // }

    // /**
    //  * @brief Generates all possible moves for the current board state
    //  * @return The number of moves generated
    //  */
    // int ManagerImpl::generateMoves()
    // {
        

        // if (state != GameState::Normal)
        //     return 0;

        // dtimer_t timer;
        // start_timer(timer);

        // this->n_moves = 0;
        // int n_pseudo_moves = 0;
        // int* iboard = this->board->getBoard();
        // bool is_white = board->getSide() == Piece::White;

        // std::unique_ptr<int[]> pseudo_moves(new int[256]);
        
        // for(int i = 0; i < 64; i++){
        //     // reset the attacks_from array & attacks_to bitboards
        //     attacks_from[0][i] = 0; 
        //     attacks_from[1][i] = 0; 
        //     attacks_to[0][i] = 0;
        //     attacks_to[1][i] = 0;
        // }

        // // Generate pseudo-legal moves
        // uint64_t occupied = board->occupied();
        // uint64_t blockers = board->occupied(is_white);
        // int king = bitScanForward(board->bitboards(is_white)[Piece::King - 1]);

        // // First generate for the opposite side with the king removed (for xRay attacks)
        // generatePseudoMoves(!is_white, occupied ^ (1ULL << king), blockers ^ (1ULL << king), nullptr);
        // n_pseudo_moves = generatePseudoMoves(is_white, occupied, board->occupied(!is_white), pseudo_moves.get());   

        // dlogln("Checking castle rights");

        // // Check for castling
        // if (board->castlingRights()){
        //     // Get starting position for the king
        //     int king = castling_data[is_white][0]; // 0 black, 1 white
        //     if (iboard[king] == Piece::getKing(castling_data[is_white][3])){
        //         dlogf("Found valid king at %s\n", square_to_str(king).c_str());
        //         // Check if rooks have moved / still have castling rights
        //         for(int j = 0; j < 2; j++){
        //             checkKingCastling(is_white, j, king);
        //         }
        //     }
        // }

        // // TODO: Implement pinned pieces
        // // Source: https://www.chessprogramming.org/Checks_and_Pinned_Pieces_(Bitboards)
        // uint64_t pinned = 0;
        // uint64_t pinners = 0;
        // uint64_t in_between_bb = 0;
        // uint64_t pinner = xRayRookAttacks(occupied, blockers, king) & board->oppRooksQueens(is_white);
        // pinners |= pinner;

        // while(pinner){
        //     int sq = bitScanForward(pinner);
        //     pinned |= Board::in_between[sq][king] & blockers;
        //     in_between_bb |= Board::in_between[sq][king];
        //     pinner &= pinner - 1;
        // }
        // pinner = xRayBishopAttacks(occupied, blockers, king) & board->oppBishopsQueens(is_white);
        // pinners |= pinner;

        // while(pinner){
        //     int sq = bitScanForward(pinner);
        //     pinned |= Board::in_between[sq][king] & blockers;
        //     in_between_bb |= Board::in_between[sq][king];
        //     pinner &= pinner - 1;
        // }

        // // validate pseudo moves
        // for(int i = 0; i < n_pseudo_moves; i++){
        //     Move move(pseudo_moves[i]);
        //     // If the piece is not the same color as the side to move, skip the move
        //     if (Piece::getColor(iboard[move.getFrom()]) != board->getSide())
        //         continue;
            
        //     if (validateMove(move, king, is_white, pinned, pinners, in_between_bb)){
        //         addMove(move.getFrom(), move.getTo(), move.getFlags(), (int*)(move_list.data()), n_moves);
        //     }
        // }

        // // Evaluate the game state
        // state = evalState();
        
        // end_timer(timer, "Move generation");
        // return n_moves;
    // }

    // /**
    //  * @brief Generates pseudo-legal moves for a given side
    //  * @param is_white True if the side is white, false otherwise
    //  * @param occupied The bitboard of the occupied squares
    //  * @param enemy_pieces The bitboard of the enemy pieces
    //  * @param move_list The list of moves to append to
    //  * @return The number of pseudo-legal moves generated, if the current side is not the same as the side to generate moves for
    //  * it will only save moves to the attacks table (`attacks_to`, `attacks_from`)
    //  */
    // int ManagerImpl::generatePseudoMoves(bool is_white, uint64_t occupied, uint64_t enemy_pieces, int* move_list)
    // {
    //     // Pawn moves
    //     uint64_t pawns = board->bitboards(is_white)[Piece::Pawn - 1];
    //     bool current_side = (board->getSide() == Piece::White) == is_white;
    //     int n_pseudo_moves = 0;

    //     // Special ranks for double pawn moves & promotions
    //     const int special_ranks[2] = {
    //         1, 6, 
    //     };

    //     // Handle pawn moves
    //     while(pawns){
    //         int from = bitScanForward(pawns);
    //         int rank = from >> 3;
    //         uint64_t attacks = Board::pawnAttacks[is_white][from];

    //         // Check for captures
    //         while(attacks){
    //             int n = bitScanForward(attacks);
    //             addAttack(from, n, is_white);
    //             if (current_side && (((1ULL << n) & enemy_pieces) != 0)){
    //                 if (rank == special_ranks[!is_white]){
    //                     // Check for promotions
    //                     addMove(from, n, Move::FLAG_ROOK_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
    //                     addMove(from, n, Move::FLAG_QUEEN_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
    //                     addMove(from, n, Move::FLAG_BISHOP_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
    //                     addMove(from, n, Move::FLAG_KNIGHT_PROMOTION_CAPTURE, move_list, n_pseudo_moves);
    //                 } else {
    //                     // Just a normal capture
    //                     addMove(from, n, Move::FLAG_CAPTURE, move_list, n_pseudo_moves);
    //                 }
    //             }                
    //             attacks &= (attacks - 1);
    //         }

    //         if (!current_side){
    //             pawns &= (pawns - 1);
    //             continue;
    //         }

    //         // Check for enpassant
    //         if((board->enpassantTarget() != 0) && (1ULL << board->enpassantTarget()) & Board::pawnAttacks[is_white][from]){
    //             addMove(from, board->enpassantTarget(), Move::FLAG_ENPASSANT_CAPTURE, move_list, n_pseudo_moves);
    //         }

    //         // Check for pawn moves
    //         int n = Board::mailbox[Board::mailbox64[from] + Board::pawn_move_offsets[is_white][0]];
    //         if (n != -1 && (occupied & (1ULL << n)) == 0){
                
    //             // Check if that's a promotion move
    //             if (rank == special_ranks[!is_white]){
    //                 addMove(from, n, Move::FLAG_ROOK_PROMOTION, move_list, n_pseudo_moves);
    //                 addMove(from, n, Move::FLAG_QUEEN_PROMOTION, move_list, n_pseudo_moves);
    //                 addMove(from, n, Move::FLAG_BISHOP_PROMOTION, move_list, n_pseudo_moves);
    //                 addMove(from, n, Move::FLAG_KNIGHT_PROMOTION, move_list, n_pseudo_moves);
    //             } else {
    //                 addMove(from, n, Move::FLAG_NONE, move_list, n_pseudo_moves);
    //             }                

    //             if (rank == special_ranks[is_white]){
    //                 // Check for double pawn moves
    //                 n = Board::mailbox[Board::mailbox64[from] + Board::pawn_move_offsets[is_white][1]];
    //                 if ((occupied & (1ULL << n)) == 0){
    //                     addMove(from, n, Move::FLAG_DOUBLE_PAWN, move_list, n_pseudo_moves);
    //                 }
    //             }
    //         }

    //         pawns &= (pawns - 1);
    //     }

    //     for(int type = 1; type < 6; type++){
    //         uint64_t pieces = board->bitboards(is_white)[type];
    //         // Current side generation (adding to the move list)
    //         if(current_side){
    //             while(pieces){
    //                 int from = bitScanForward(pieces);
    //                 for(int rays = 0; rays < Board::n_piece_rays[type]; rays++){
    //                     for(int n = from;;){
    //                         n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][rays]];
    //                         if (n == -1){
    //                             break;
    //                         }
    //                         uint64_t bitmove = 1ULL << n;
    //                         addAttack(from, n, is_white);
    //                         if (occupied & bitmove){
    //                             if (enemy_pieces & bitmove){
    //                                 addMove(from, n, Move::FLAG_CAPTURE, move_list, n_pseudo_moves);
    //                             }
    //                             break;
    //                         }

    //                         addMove(from, n, Move::FLAG_NONE, move_list, n_pseudo_moves);
    //                         if (!Board::is_piece_sliding[type]){
    //                             break;
    //                         }
    //                     }
    //                 }
    //                 pieces &= (pieces - 1);
    //             }
    //         } else {
    //             // Enemy pieces generation (only adding to the attacks table)
    //             while(pieces){
    //                 int from = bitScanForward(pieces);
    //                 for(int rays = 0; rays < Board::n_piece_rays[type]; rays++){
    //                     for(int n = from;;){
    //                         n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][rays]];
    //                         if (n == -1){
    //                             break;
    //                         }
    //                         addAttack(from, n, is_white);
    //                         if (occupied & (1ULL << n)){
    //                             break;
    //                         }
    //                         if (!Board::is_piece_sliding[type]){
    //                             break;
    //                         }
    //                     }
    //                 }
    //                 pieces &= (pieces - 1);
    //             }                
    //         }
    //     }

    //     return n_pseudo_moves;
    // }

    // bool ManagerImpl::validateMove(Move& move, int king, bool is_white, uint64_t pinned, uint64_t pinners, uint64_t in_between_bb)
    // {
    //     int to = move.getTo();
    //     int from = move.getFrom();
    //     uint64_t king_attack = attacks_to[!is_white][king];
    //     const int enpassant_dir[2] = {-8, 8};

    //     if (from != king){
    //         // Check if the king is not in check
    //         if (king_attack == 0){
    //             // Check if the piece is not pinned
    //             uint64_t bitmove = 1ULL << from;

    //             // Check if the move is enpassant (special pawn case)
    //             if (move.isEnPassant()){
    //                 int rank = (from >> 3) << 3;
    //                 uint64_t oppRQ = board->oppRooksQueens(is_white);
    //                 if(Board::in_between[rank][rank + 7] & oppRQ){
    //                     // Check with x-ray attacks, without the pawn target
    //                     uint64_t occupied = board->occupied() ^ (1ULL << (to + enpassant_dir[is_white]));
    //                     uint64_t blockers = board->occupied(is_white);
    //                     uint64_t pinner = xRayRookAttacks(occupied, blockers, king) & board->oppRooksQueens(is_white);
    //                     while(pinner){
    //                         int sq = bitScanForward(pinner);
    //                         pinned |= Board::in_between[sq][king] & blockers;
    //                         pinner &= pinner - 1;
    //                     }
    //                 }
    //             }

    //             if (pinned & bitmove){
    //                 // The piece is pinned, so it can only move along the pin line or capture the pinning piece
    //                 return ((in_between_bb | pinners) & (1ULL << to)) != 0;
    //             }               

    //             return true;
    //         }
            
    //         // If there is more than one piece attacking the king, the move is invalid
    //         // or the piece is pinned it cannot capture the checking piece
    //         if (king_attack & (king_attack - 1) || pinned & (1ULL << from)){
    //             // More than one piece is attacking the king
    //             return false;
    //         }

    //         int attacker = bitScanForward(king_attack);
    //         const int offsets[2] = {0, enpassant_dir[is_white]};
    //         int offset = offsets[move.isEnPassant()];

    //         // Check if the move can block the check or capture the attacking piece
    //         return (Board::in_between[attacker][king] | king_attack) & (1ULL << (to + offset));
    //     }

    //     // King move, so the square must not be attacked by the enemy
    //     return attacks_to[!is_white][to] == 0;
    // }

    // /**
    //  * @brief Check if the king can castle to the given side.
    //  * @param is_white True if the king is white, false otherwise
    //  * @param j The index of the castling_data array at position 1 or 2
    //  * @param king The index of the king
    //  */
    // void ManagerImpl::checkKingCastling(bool is_white, int j, int king){
    //     int *iboard = board->board.get();

    //     int rook = iboard[castling_data[is_white][j + 1]];
    //     if (rook != Piece::getRook(castling_data[is_white][3])) // not the correct piece (rook)
    //         return;

    //     const int castling_rights[2][2] = {
    //         {CastlingRights::BLACK_QUEEN, CastlingRights::BLACK_KING},
    //         {CastlingRights::WHITE_QUEEN, CastlingRights::WHITE_KING}
    //     };

    //     // Check if we have correct castling rights
    //     if(!board->castlingRights().has(castling_rights[is_white][j]))
    //         return;
        
    //     dlogf("Valid rook at %s\n", square_to_str(castling_data[is_white][j + 1]).c_str());
    //     if (attacks_to[!is_white][king] != 0) // king in check
    //         return;

    //     // Check if the squares between the king and the rook are empty and 
    //     // not attacked by the enemy color
    //     int pos = king;
    //     int rook_pos = castling_data[is_white][j + 1];
    //     int target_king_pos = king + castling_offsets[0][j];

    //     while(pos != target_king_pos){
    //         pos += castling_offsets[1][j];
    //         dlogf("Checking position at %s\n", square_to_str(pos).c_str());
    //         dbitboard(attacks_to[!is_white][pos]);
    //         if (iboard[pos] != Piece::Empty || attacks_to[!is_white][pos] != 0) // piece in the way or attacked
    //             return;
    //     }

    //     while(pos != rook_pos){
    //         if (iboard[pos] != Piece::Empty)
    //             return;
    //         pos += castling_offsets[1][j];
    //     }

    //     // valid castle
    //     dlogf("Added possible castle at %s\n", square_to_str(target_king_pos).c_str());
    //     addMove(king, target_king_pos, castling_flags[j], (int*)move_list.data(), n_moves);
    // }
    // /**
    //  * @brief Adds a move to the move list
    //  */
    // void ManagerImpl::addMove(int from, int to, int flags, int* move_list, int& n_moves){
    //     move_list[n_moves++] = int(Move(from, to, flags));
    // }
    // /**
    //  * @brief Updates `attacks_to` and `attacks_from` bitboards
    //  * @param from The square from which the attack is coming
    //  * @param to The square being attacked
    //  * @param is_white True if the attacking piece is white, false otherwise
    //  */
    // void ManagerImpl::addAttack(int from, int to, bool is_white){
    //     // basically, attacks_to[is_white][x] is usefull if you want to check how many pieces are attacking 
    //     // given square x, and attacks_from[is_white][x] if you want to check how many squares are being attacked
    //     // by the piece in square x. The `is_white` variable is used to differentiate between white and black pieces
    //     attacks_to[is_white][to] |= 1UL << from; 
    //     attacks_from[is_white][from] |= 1UL << to;
    // }
}