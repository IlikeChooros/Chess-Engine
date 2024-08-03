#include <cengine/move_gen.h>

/**
 * @brief Initialize the board, generate the in_between array and piece attacks bitboards
 */
void init_board(chess::Board* board)
{
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

    using namespace chess;

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
            Board::in_between[i][j] = line & btwn;   /* return the bits on that line in-between */
        }
    }

    // Init piece attacks
    for(int i = 0; i < 64; i++){
        Board::knightAttacks[i] = mailboxAttacks(Piece::Knight - 1, 0, i, false);
        Board::kingAttacks[i] = mailboxAttacks(Piece::King - 1, 0, i, false);
        Board::queenAttacks[i] = mailboxAttacks(Piece::Queen - 1, 0, i, true);

        // Pawn attacks
        for(int j = 0; j < 2; j++){
            for(int k = 0; k < 2; k++){
                int n = Board::mailbox[Board::mailbox64[i] + Board::pawn_attack_offsets[j][k]];
                if(n != -1){
                    Board::pawnAttacks[j][i] |= 1ULL << n;
                }
            }
        }
    }
}

/**
 * @brief Validate the castling rights, it may delete the castling rights 
 * if the rooks or the king are not in the correct position
 */
void verify_castling_rights(chess::Board* board)
{
    using namespace chess;
    const int castling_data[2][3] = {
        {4, 0, 7}, // black king data: starting position, rook positions
        {60, 56, 63} // white king (same as above)
    };
    const int kings[2] = {
        bitScanForward(board->bitboards(false)[Piece::King - 1]),
        bitScanForward(board->bitboards(true)[Piece::King - 1])
    };
    const int castling_rights[2][2] = {
        {CastlingRights::BLACK_QUEEN, CastlingRights::BLACK_KING},
        {CastlingRights::WHITE_QUEEN, CastlingRights::WHITE_KING}
    };
    int* iboard = board->getBoard();
    CastlingRights cr(board->castlingRights().get());

    // Check if the rooks are in the correct position
    for (int is_white = 0; is_white < 2; is_white++){
        for(int i = 0; i < 2; i++){
            int rook_target = castling_data[is_white][i + 1];
            if (Piece::getType(iboard[rook_target]) != Piece::Rook){
                cr.remove(castling_rights[is_white][i]);
            }
        }
    }
    // Check if the king is in the correct position
    for (int is_white = 0; is_white < 2; is_white++){
        if (kings[is_white] != castling_data[is_white][0]){
            cr.remove(castling_rights[is_white][0]);
            cr.remove(castling_rights[is_white][1]);
        }
    }

    board->m_castling_rights = cr;
}

/**
 * @brief Generates the attacks for a piece
 * @author https://www.chessprogramming.org/10x12_Board 
 * @param type The type of the piece (Piece::Rook - 1, Piece::Bishop - 1, Piece::Queen - 1, Piece::Knight - 1, Piece::King - 1)
 * @param occupied The bitboard of the occupied squares
 * @param square The square of the piece
 */
uint64_t mailboxAttacks(int type, uint64_t occupied, int square, bool is_sliding)
{
    using namespace chess;
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

/**
 * @brief Generate pawn attacks for a given square
 */
uint64_t mailboxPawnMoves(uint64_t occupied, int square, bool is_white)
{
    using namespace chess;
    const int ranks[2] = {1, 6};
    bool is_special_rank = (square >> 3) == ranks[is_white];
    uint64_t moves = 0;
    for(int j = 0; j < 2; j++){
        int n = Board::mailbox[Board::mailbox64[square] + Board::pawn_move_offsets[is_white][j]];

        // Check if the square is outside of the board or if it's occupied
        if (n == -1 || occupied & (1ULL << n)){
            break;
        }
        moves |= 1ULL << n;

        // Break if the pawn is not on the 2nd (white) or 7th (black) rank
        if(!is_special_rank){
            break;
        }
    }
    return moves;
}

/**
 * @brief Get the rook attacks for a given square, attacks are both squares attacked and pieces
 */
uint64_t rookAttacks(uint64_t occupied, int rook)
{
    using namespace chess;
    return mailboxAttacks(Piece::Rook - 1, occupied, rook, true);
}

/**
 * @brief Get the bishop attacks for a given square
 */
uint64_t bishopAttacks(uint64_t occupied, int bishop)
{
    using namespace chess;
    return mailboxAttacks(Piece::Bishop - 1, occupied, bishop, true);
}

/**
 * @brief Get the x-ray attacks for a rook
 * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
 */
uint64_t xRayRookAttacks(uint64_t occupied, uint64_t blockers, int rooksq)
{
    uint64_t attacks = rookAttacks(occupied, rooksq); // get the valid rook attacks (all pieces involved)
    blockers &= attacks; // take only the blockers that are in the attacks (attacked blockers)
    return attacks ^ rookAttacks(occupied ^ blockers, rooksq); // get the x-ray attacks (attacks through the blockers)
}

/**
 * @brief Get the x-ray attacks for a bishop
 * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
 */
uint64_t xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int bishopsq)
{
    uint64_t attacks = bishopAttacks(occupied, bishopsq);
    blockers &= attacks;
    return attacks ^ bishopAttacks(occupied ^ blockers, bishopsq);
}

/**
 * @brief Make a move on the board, updates the board state (board array, bitboards, side to move, etc.)
 * @param move The move to make 
 * @param board The board to update
 */
void make(Move& move, chess::Board* board, chess::GameHistory* ghistory)
{
    using namespace chess;

    int* iboard = board->board.get();
    int to = move.getTo();
    int from = move.getFrom();
    bool is_white = Piece::isWhite(iboard[from]);

    // Update fullmove counter
    if (board->getSide() == Piece::Black){
        board->fullmoveCounter()++;
    }

    // Update halfmove clock, if the move is a pawn move or a capture, reset the clock
    if (Piece::getType((*board)[from]) == Piece::Pawn){
        board->halfmoveClock() = 0;
    } else {
        board->halfmoveClock()++;
    }

    board->capturedPiece() = Piece::Empty; // Set as empty, it may change if the move is a capture
    // If that's a capture, remove the captured piece
    if (move.isCapture()){
        board->m_halfmove_clock = 0;
        int offset = 0;

        if(move.isEnPassant()){
            offset = Piece::isWhite(iboard[from]) ? 8 : -8;
        }
        board->capturedPiece() = iboard[to + offset];
        iboard[to + offset] = Piece::Empty;

        // Update the bitboard for the captured piece
        board->bitboards(Piece::isWhite(board->capturedPiece()))[Piece::getType(board->capturedPiece()) - 1] ^= 1ULL << (to + offset);
    }

    // Reset the enpassant target
    board->enpassantTarget() = 0;

    // If the move is a castle, move the rook
    if(move.isCastle()){
        bool is_king_castle = move.isKingCastle();

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

        // Move the rook (king will be handled outside of this block)
        iboard[rook_to] = iboard[rook_from]; 
        iboard[rook_from] = Piece::Empty;

        // Update the bitboard for the rook
        board->updateBitboard(is_white, Piece::Rook - 1, rook_from, rook_to);
    }

    // If the move is a promotion, set enpassant target
    if (move.isDoubleMove()){
        const int dir[2] = {-8, 8};
        board->enpassantTarget() = to + dir[is_white];
    }

    // If that's a promotion, update the piece
    if (move.isPromotion()){
        const int promotion_piece[4] = { Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen };
        iboard[from] = promotion_piece[move.getPromotionPiece()] | (board->getSide()); // set the piece with the correct color
        board->bitboards(is_white)[Piece::Pawn - 1] ^= 1ULL << from; // remove the pawn
        board->bitboards(is_white)[Piece::getType(iboard[from]) - 1] |= 1ULL << from; // add the promoted piece (not moved yet)
    }

    // Update castling rights & king position
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
    dbitboard(board->bitboards(is_white)[Piece::getType(iboard[to]) - 1]);

    // Now update the side to move
    board->getSide() = Piece::opposite(board->getSide());
    ghistory->push(board, move);
}

/**
 * @brief Unmake a move on the board, updates the board state (board array, bitboards, side to move, etc.)
 * @param move The move to unmake
 * @param board The board to update
 * @param ghistory The game history to pop the last move
 */
void unmake(Move& move, chess::Board* board, chess::GameHistory* ghistory)
{
    using namespace chess;

    if(ghistory->size() <= 1)
        return;

    // Pop current move from history & get the previous one
    ghistory->pop();
    auto history = ghistory->back();   
    int* iboard = board->getBoard();
    int from = move.getFrom(), to = move.getTo(), captured_pos = to;
    bool is_white = history.side_to_move == Piece::White;
    const int colors[2] = { Piece::Black, Piece::White };

    if (move.isCapture()){
        int offset = 0;
        if(move.isEnPassant()){
            const int dir[2] = {-8, 8};
            offset = dir[is_white];
        }
        captured_pos += offset;
        // Restore the captured piece in bitboards
        board->bitboards(!is_white)[Piece::getType(board->capturedPiece()) - 1] |= 1ULL << captured_pos;
    } else if (move.isCastle()){
        const int rooks_to[2][2] = {
            {0, 56}, // queen castle
            {7, 63} // king castle
        };
        const int rooks_from[2][2] = {
            {3, 59}, // queen castle
            {5, 61} // king castle
        };
        // Get rook starting position
        bool is_king_castle = move.isKingCastle();
        int rook_from = rooks_from[is_king_castle][is_white];
        int rook_to = rooks_to[is_king_castle][is_white];
        iboard[rook_to] = iboard[rook_from]; // move the rook back
        iboard[rook_from] = Piece::Empty; // empty the rook's previous position
        board->updateBitboard(is_white, Piece::Rook - 1, rook_from, rook_to); // update the bitboard
    }

    // Restore the promotion piece
    if (move.isPromotion()){
        board->bitboards(is_white)[Piece::getType(iboard[to]) - 1] ^= 1ULL << to; // remove the promoted piece
        iboard[to] = Piece::Pawn | colors[is_white]; // restore the pawn
        board->bitboards(is_white)[Piece::Pawn - 1] |= 1ULL << to; // add the pawn to the bitboard (not moved yet)
    }

    // Restore the moved piece
    iboard[from] = iboard[to];
    iboard[to] = Piece::Empty;
    board->updateBitboard(is_white, Piece::getType(iboard[from]) - 1, to, from);

    // Restore the captured piece
    iboard[captured_pos] = board->capturedPiece();

    // restore other variables
    board->getSide() = history.side_to_move;
    board->halfmoveClock() = history.halfmove_clock;
    board->enpassantTarget() = history.enpassant_target;
    board->castlingRights() = history.castling_rights;
    board->fullmoveCounter() = history.fullmove_counter;
    board->capturedPiece() = history.captured_piece;
    move = history.move;
}

// Generate all possible moves for the current board state, board should be initialized
size_t gen_legal_moves(MoveList* moves, chess::Board* board){
    using namespace chess;

    bool is_white = board->getSide() == Piece::White;
    bool is_enemy = !is_white;
    uint64_t occupied = board->occupied();
    uint64_t occupied_noking = occupied ^ board->bitboards(is_white)[Piece::King - 1];
    uint64_t enemy_pieces = board->occupied(!is_white);
    uint64_t allied_pieces = occupied ^ enemy_pieces;
    int king = bitScanForward(board->bitboards(is_white)[Piece::King - 1]);
    moves->clear();

    // Bitboard for the enemy attacks
    uint64_t bitboard;
    uint64_t danger = 0;

    // Generate attacks for enemy bishops
    bitboard = board->bitboards(is_enemy)[Piece::Bishop - 1];
    while(bitboard) danger |= bishopAttacks(occupied_noking, pop_lsb1(bitboard));
    

    // Generate attacks for enemy rooks
    bitboard = board->bitboards(is_enemy)[Piece::Rook - 1];
    while (bitboard) danger |= rookAttacks(occupied_noking, pop_lsb1(bitboard));
    

    // Generate attacks for enemy knights
    bitboard = board->bitboards(is_enemy)[Piece::Knight - 1];
    while (bitboard) danger |= Board::knightAttacks[pop_lsb1(bitboard)];
    

    // Generate attacks for enemy queen(s)
    bitboard = board->bitboards(is_enemy)[Piece::Queen - 1];
    while(bitboard){
        int sq = pop_lsb1(bitboard);
        danger |= rookAttacks(occupied_noking, sq) | bishopAttacks(occupied_noking, sq);
    }

    // Generate attacks for enemy pawns
    bitboard = board->bitboards(is_enemy)[Piece::Pawn - 1];
    while(bitboard) danger |= Board::pawnAttacks[is_enemy][pop_lsb1(bitboard)];
    
    // Generate attacks for enemy king
    danger |= Board::kingAttacks[bitScanForward(board->bitboards(is_enemy)[Piece::King - 1])];

    // Now generate pins
    // Source: https://www.chessprogramming.org/Pinned_Pieces
    uint64_t pinned = 0;
    uint64_t pinners = 0;
    uint64_t pinner = xRayRookAttacks(occupied, allied_pieces, king) & board->oppRooksQueens(is_white);
    pinners |= pinner;

    while(pinner) pinned |= Board::in_between[pop_lsb1(pinner)][king] & allied_pieces;

    pinner = xRayBishopAttacks(occupied, allied_pieces, king) & board->oppBishopsQueens(is_white);
    pinners |= pinner;

    while(pinner) pinned |= Board::in_between[pop_lsb1(pinner)][king] & allied_pieces;
    

    // See if the king is in check
    if (danger & board->bitboards(is_white)[Piece::King - 1]){
        // Check the number of attackers
        uint64_t attackers = 0;
        attackers |= bishopAttacks(occupied, king) & board->oppBishopsQueens(is_white);
        attackers |= rookAttacks(occupied, king) & board->oppRooksQueens(is_white);
        attackers |= Board::knightAttacks[king] & board->bitboards(is_enemy)[Piece::Knight - 1];
        attackers |= Board::pawnAttacks[is_white][king] & board->bitboards(is_enemy)[Piece::Pawn - 1];

        // If there are more than one attackers, the king is in double check, only king moves are allowed
        if (attackers & (attackers - 1)){
            // Generate king moves
            // King can only move to evade the check
            uint64_t kmoves = Board::kingAttacks[king] & ~occupied & ~danger;
            uint64_t captures = Board::kingAttacks[king] & enemy_pieces & ~danger;

            while(kmoves) moves->add(Move(king, pop_lsb1(kmoves), Move::FLAG_NONE).move());
            while(captures) moves->add(Move(king, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
            
            return moves->size();
        }

        // Generate moves to block the check,
        // The things to look out for when generating moves:
        // - My pieces might be pinned:
        //   - pinned pieces cannot move when the king is in check (them moving would expose the king)
        // - Pieces can only either capture the attacker or block the path
        // - Enpassant is possible only if the attacker is a pawn
        // - King may only move to evade the check
        int attackers_sq = bitScanForward(attackers);
        uint64_t block_path = Board::in_between[attackers_sq][king]; // squares in-between the attacker
        uint64_t block_moves = 0;

        // Generate moves for not pinned pieces
        uint64_t not_pinned = ~pinned & allied_pieces;
        
        // Generate moves for bishops
        bitboard = board->bitboards(is_white)[Piece::Bishop - 1] & not_pinned;
        while(bitboard){
            int sq = pop_lsb1(bitboard);
            block_moves = bishopAttacks(occupied, sq);
            uint64_t captures = block_moves & attackers;
            block_moves &= ~occupied & block_path;
            // If that's a capture, add the move
            if (captures){
                moves->add(Move(sq, attackers_sq, Move::FLAG_CAPTURE).move());
            }

            while(block_moves){
                moves->add(Move(sq, pop_lsb1(block_moves), Move::FLAG_NONE).move());
            }
        }

        // Generate moves for rooks
        bitboard = board->bitboards(is_white)[Piece::Rook - 1] & not_pinned;
        while(bitboard){
            int sq = pop_lsb1(bitboard);
            block_moves = rookAttacks(occupied, sq);
            uint64_t captures = block_moves & attackers;
            block_moves &= ~occupied & block_path;

            // If that's a capture, add the move
            if (captures){
                moves->add(Move(sq, attackers_sq, Move::FLAG_CAPTURE).move());
            }

            while(block_moves){
                moves->add(Move(sq, pop_lsb1(block_moves), Move::FLAG_NONE).move());
            }
        }

        // Generate moves for queens
        bitboard = board->bitboards(is_white)[Piece::Queen - 1] & not_pinned;
        while(bitboard){
            int sq = pop_lsb1(bitboard);
            block_moves = (rookAttacks(occupied, sq) | bishopAttacks(occupied, sq));
            uint64_t captures = block_moves & attackers;
            block_moves &= ~occupied & block_path;
            // If that's a capture, add the move
            if (captures){
                moves->add(Move(sq, attackers_sq, Move::FLAG_CAPTURE).move());
            }

            while(block_moves){
                moves->add(Move(sq, pop_lsb1(block_moves), Move::FLAG_NONE).move());
            }
        }

        // Generate moves for knights
        bitboard = board->bitboards(is_white)[Piece::Knight - 1] & not_pinned;
        while(bitboard){
            int sq = pop_lsb1(bitboard);
            block_moves = Board::knightAttacks[sq];
            uint64_t captures = block_moves & attackers;
            block_moves &= ~occupied & block_path;
            // If that's a capture, add the move
            if (captures){
                moves->add(Move(sq, attackers_sq, Move::FLAG_CAPTURE).move());
            }

            while(block_moves){
                moves->add(Move(sq, pop_lsb1(block_moves), Move::FLAG_NONE).move());
            }
        }

        // Generate moves for pawns
        // Enpassant is possible when the king is in check (only if the attacker is a pawn)
        bitboard = board->bitboards(is_white)[Piece::Pawn - 1] & not_pinned;
        const int offset[2] = {-8, 8};
        const int ranks[2] = {1, 6};
        while(bitboard){
            int sq = pop_lsb1(bitboard);
            uint64_t captures = Board::pawnAttacks[is_white][sq] & attackers;
            uint64_t enpassant = board->enpassantTarget() ? Board::pawnAttacks[is_white][sq] & (1ULL << (board->enpassantTarget())) : 0;
            int rank = sq >> 3;

            // If the enpassant is possible and the attacker is on a valid capture square, then add the move
            if (enpassant && attackers & (1ULL << (board->enpassantTarget() + offset[is_white]))){
                moves->add(Move(sq, board->enpassantTarget(), Move::FLAG_ENPASSANT_CAPTURE).move());
            }
            // If that's a capture, add the move
            if (captures){
                // Generate captures promoting moves (the pawn is on the either 2nd or 7th rank)
                if (rank == ranks[is_enemy]){
                    moves->add(Move(sq, attackers_sq, Move::FLAG_ROOK_PROMOTION_CAPTURE).move());
                    moves->add(Move(sq, attackers_sq, Move::FLAG_BISHOP_PROMOTION_CAPTURE).move());
                    moves->add(Move(sq, attackers_sq, Move::FLAG_KNIGHT_PROMOTION_CAPTURE).move());
                    moves->add(Move(sq, attackers_sq, Move::FLAG_QUEEN_PROMOTION_CAPTURE).move());
                } else {
                    moves->add(Move(sq, attackers_sq, Move::FLAG_CAPTURE).move());
                }
            }

            // Generate normal pawn moves
            int n = Board::mailbox[Board::mailbox64[sq] + Board::pawn_move_offsets[is_white][0]];
            uint64_t pmoves = (1ULL << n) & ~occupied;

            // If the pawn push is blocked, stop here
            if(!pmoves)
                continue;

            if (pmoves & block_path){
                // If the pawn is on the 2nd (black) or 7th rank (white), generate promotion moves
                if (rank == ranks[!is_white]){
                    int to = bitScanForward(pmoves);
                    moves->add(Move(sq, to, Move::FLAG_ROOK_PROMOTION).move());
                    moves->add(Move(sq, to, Move::FLAG_BISHOP_PROMOTION).move());
                    moves->add(Move(sq, to, Move::FLAG_KNIGHT_PROMOTION).move());
                    moves->add(Move(sq, to, Move::FLAG_QUEEN_PROMOTION).move());
                    continue;
                }      
                moves->add(Move(sq, n, Move::FLAG_NONE).move());
            }

            // If the pawn is on the 2nd (white) or 7th rank (black), generate double pawn push
            if (rank == ranks[is_white]){
                n = Board::mailbox[Board::mailbox64[sq] + Board::pawn_move_offsets[is_white][1]];
                if ((1ULL << n) & ~occupied & block_path){
                    moves->add(Move(sq, n, Move::FLAG_DOUBLE_PAWN).move());
                }
            }
        }

        // Generate moves for the king (it can only evade the check)
        uint64_t kmoves = Board::kingAttacks[king] & ~occupied & ~danger;
        uint64_t captures = Board::kingAttacks[king] & enemy_pieces & ~danger;

        while(kmoves) moves->add(Move(king, pop_lsb1(kmoves), Move::FLAG_NONE).move());
        while(captures) moves->add(Move(king, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
        
        return moves->size();
    }

    // If the king is not in check, generate all possible moves
    // The things to look out for when generating moves:
    // - My pieces might be pinned:
    //   - pinned pieces may move only along the pin line (or capture the attacker)
    //   - there may be only one pinner for each pinned piece
    //   - special pin pawn enpassant case 
    // - King cannot move the attacked square
    // - King cannot castle if the square between target position (included) and
    // starting position is attacked

    uint64_t bmoves, captures;

    // Generate moves for bishops
    bitboard = board->bitboards(is_white)[Piece::Bishop - 1];
    while(bitboard){
        int sq = pop_lsb1(bitboard);
        bmoves = bishopAttacks(occupied, sq);
        captures = bmoves & enemy_pieces; 
        bmoves &= ~occupied;

        // If the piece is pinned, restrict the moves
        if (pinned & (1ULL << sq)){
            bmoves &= Board::in_between[bitScanForward(pinners & Board::queenAttacks[sq])][king];
            captures &= pinners;
        }

        while(bmoves) moves->add(Move(sq, pop_lsb1(bmoves), Move::FLAG_NONE).move());
        while(captures) moves->add(Move(sq, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
    }

    // Generate moves for rooks
    bitboard = board->bitboards(is_white)[Piece::Rook - 1];
    while(bitboard){
        int sq = pop_lsb1(bitboard);
        bmoves = rookAttacks(occupied, sq);
        captures = bmoves & enemy_pieces;
        bmoves &= ~occupied;

        // If the piece is pinned, restrict the moves
        if (pinned & (1ULL << sq)){
            bmoves &= Board::in_between[bitScanForward(pinners & Board::queenAttacks[sq])][king];
            captures &= pinners;
        }

        while(bmoves) moves->add(Move(sq, pop_lsb1(bmoves), Move::FLAG_NONE).move());
        while(captures) moves->add(Move(sq, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
    }

    // Generate moves for queens
    bitboard = board->bitboards(is_white)[Piece::Queen - 1];
    while(bitboard){
        int sq = pop_lsb1(bitboard);
        bmoves = (rookAttacks(occupied, sq) | bishopAttacks(occupied, sq));
        captures = bmoves & enemy_pieces;
        bmoves &= ~occupied;

        // If the piece is pinned, restrict the moves
        if (pinned & (1ULL << sq)){
            bmoves &= Board::in_between[bitScanForward(pinners & Board::queenAttacks[sq])][king];
            captures &= pinners;
        }

        while(bmoves) moves->add(Move(sq, pop_lsb1(bmoves), Move::FLAG_NONE).move());
        while(captures) moves->add(Move(sq, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
    }

    // Generate moves for knights
    bitboard = board->bitboards(is_white)[Piece::Knight - 1];
    while(bitboard){
        int sq = pop_lsb1(bitboard);
        bmoves = Board::knightAttacks[sq];

        // If knight is pinned, it cannot move
        if (pinned & (1ULL << sq)){
            continue;
        }

        captures = bmoves & enemy_pieces;
        bmoves &= ~occupied;
        while(bmoves) moves->add(Move(sq, pop_lsb1(bmoves), Move::FLAG_NONE).move());
        while(captures) moves->add(Move(sq, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
    }

    // Generate moves for pawns
    bitboard = board->bitboards(is_white)[Piece::Pawn - 1];
    const int offset[2] = {8, -8};
    const int ranks[2] = {1, 6}; // board is inversed, so the ranks are different
    while(bitboard){
        int sq = pop_lsb1(bitboard);
        uint64_t enpassant_target = board->enpassantTarget() ? 1ULL << board->enpassantTarget() : 0;
        uint64_t pinline = 0;
        captures = Board::pawnAttacks[is_white][sq] & enemy_pieces;
        int rank = sq >> 3;

        // If the piece is pinned, restrict the moves
        if (pinned & (1ULL << sq)){
            pinline = Board::in_between[bitScanForward(pinners & Board::queenAttacks[sq])][king];
            captures &= pinners;
            enpassant_target &= pinline;
        }

        // Generate captures promoting moves (the pawn is on the either 2nd or 7th rank)
        if (rank == ranks[is_enemy]){
            while(captures) {
                int cap_sq = pop_lsb1(captures);
                moves->add(Move(sq, cap_sq, Move::FLAG_ROOK_PROMOTION_CAPTURE).move());
                moves->add(Move(sq, cap_sq, Move::FLAG_BISHOP_PROMOTION_CAPTURE).move());
                moves->add(Move(sq, cap_sq, Move::FLAG_KNIGHT_PROMOTION_CAPTURE).move());
                moves->add(Move(sq, cap_sq, Move::FLAG_QUEEN_PROMOTION_CAPTURE).move());
            }
        } else {
            while(captures) moves->add(Move(sq, pop_lsb1(captures), Move::FLAG_CAPTURE).move());
        }

        // Enpassant is possible only if the last move was a double pawn move
        if (enpassant_target & Board::pawnAttacks[is_white][sq]){
            // If the enpassant is possible, we should check if the pawn is pinned
            // Consider a board with enpassant target at d6
            // . . . . . . . . 8
            // . . . . . . . . 7
            // . . . x . . . . 6
            // R . P p . . . K 5
            // k . . . . . . . 4
            // . . . . . . . . 3
            // . . . . . . . . 2
            // . . . . . . . . 1
            // a b c d e f g h
            // Taking the pawn here would expose the king to the rook / queen
            // So we should check if the pawn is pinned to the king 
            // (now without the black pawn)
            uint64_t occ = occupied ^ (1ULL << (board->enpassantTarget() - offset[is_white]));
            uint64_t opRQ = board->oppRooksQueens(is_white);
            uint64_t ppiner = xRayRookAttacks(occ, allied_pieces, king) & opRQ;
            uint64_t ppinned = 0;
            while(ppiner){
                ppinned |= Board::in_between[pop_lsb1(ppiner)][king] & allied_pieces;
            }
            
            // Check again if the pawn is pinned
            if (ppinned & (1ULL << sq)){
                enpassant_target &= pinline;
            }

            // If the pawn is not pinned or it can move along the pin line, add the move
            if (enpassant_target){
                moves->add(Move(sq, board->enpassantTarget(), Move::FLAG_ENPASSANT_CAPTURE).move());
            }
        }

        // Generate normal pawn moves
        int n = Board::mailbox[Board::mailbox64[sq] + Board::pawn_move_offsets[is_white][0]];
        bmoves = (1ULL << n) & ~occupied;

        // If the pawn push is blocked, stop here
        if(!bmoves)
            continue;

        // Check if the pawn is pinned, if it is, restrict the moves
        if (pinned & (1ULL << sq)){
            bmoves &= pinline;
        }

        // this move might not be possible (because of pin)
        if (bmoves){
            // If the pawn is on the 2nd (black) or 7th rank (white), generate promotion moves
            if (rank == ranks[!is_white]){
                int to = bitScanForward(bmoves);
                moves->add(Move(sq, to, Move::FLAG_ROOK_PROMOTION).move());
                moves->add(Move(sq, to, Move::FLAG_BISHOP_PROMOTION).move());
                moves->add(Move(sq, to, Move::FLAG_KNIGHT_PROMOTION).move());
                moves->add(Move(sq, to, Move::FLAG_QUEEN_PROMOTION).move());
                continue;
            }
            moves->add(Move(sq, n, Move::FLAG_NONE).move());
        }

        // If the pawn is on the 2nd (white) or 7th rank (black), generate double pawn push
        if (rank == ranks[is_white]){
            n = Board::mailbox[Board::mailbox64[sq] + Board::pawn_move_offsets[is_white][1]];
            bmoves = (1ULL << n) & ~occupied;

            // Check if the pawn is pinned, if it is, restrict the moves
            if (pinned & (1ULL << sq)){
                bmoves &= pinline;
            }

            // this move might not be possible (because of pin)
            if (bmoves){
                moves->add(Move(sq, n, Move::FLAG_DOUBLE_PAWN).move());
            }
        }
    }

    // Generate moves for the king
    bmoves = Board::kingAttacks[king] & ~occupied & ~danger;
    captures = Board::kingAttacks[king] & enemy_pieces & ~danger;

    while(bmoves) moves->add(Move(king, pop_lsb1(bmoves), Move::FLAG_NONE).move());
    while(captures) moves->add(Move(king, pop_lsb1(captures), Move::FLAG_CAPTURE).move());

    // Generate castling moves, I assume that castling rights are correct
    // Hence I always verify the castling rights after reloading the manager and on creating one
    verify_castling_rights(board);
    const uint32_t colors[2] = { CastlingRights::BLACK, CastlingRights::WHITE };
    if (board->castlingRights().has(colors[is_white])){
        bool king_side = (board->castlingRights().getKing() & colors[is_white]) != 0;
        bool queen_side = (board->castlingRights().getQueen() & colors[is_white]) != 0;
        uint64_t queen_path = 0b00001100, king_path = 0b01100000;
        uint64_t queen_path_no_occ = 0b00001110;

        if(is_white){
            queen_path <<= 56;
            king_path <<= 56;
            queen_path_no_occ <<= 56;
        }

        // King can castle safely only if the square between target position and starting position
        // aren't occupied and aren't attacked
        if (king_side && (king_path & (~occupied) & (~danger)) == king_path){
            moves->add(Move(king, king + 2, Move::FLAG_KING_CASTLE).move());
        }

        // Additionally space between rook and king should be empty (in king's side case `king_path` = `king_path_no_occ`)
        if (queen_side && (queen_path & (~occupied) & (~danger)) == queen_path && (queen_path_no_occ & (~occupied)) == queen_path_no_occ){
            moves->add(Move(king, king - 2, Move::FLAG_QUEEN_CASTLE).move());
        }
    }

    return moves->size();
}