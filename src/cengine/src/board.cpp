#include <cengine/board.h>


namespace chess
{

    // Helper bitboards
    Bitboard Board::in_between[64][64]  = {0};
    Bitboard Board::pawnAttacks[2][64]  = {0};
    Bitboard Board::pieceAttacks[6][64] = {0};

    // Starting position
    const char Board::START_FEN[57] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board::Board()
    {
        memset(board, Piece::Empty, sizeof(board));
        for(int i = 0; i < 2; i++)
            memset(m_bitboards[i], 0, sizeof(m_bitboards[i]));
        
        m_history.reserve(64);

        m_side               = Piece::White;
        m_halfmove_clock     = 0;
        m_fullmove_counter   = 1;
        m_enpassant_target   = 0;
        m_castling_rights    = CastlingRights::ALL;
        m_captured_piece     = Piece::Empty;
        m_irreversible_index = 0;
        m_in_check           = false;
    }

    Board::Board(const Board& other)
    {
        *this = other;
    }

    Board& Board::operator=(const Board& other)
    {
        // Copy the board, field order is kept

        m_hash              = other.m_hash;
        memcpy(board, other.board, sizeof(board));
        m_in_check           = other.m_in_check;
        m_side               = other.m_side;
        m_enpassant_target   = other.m_enpassant_target;
        m_halfmove_clock     = other.m_halfmove_clock;
        m_fullmove_counter   = other.m_fullmove_counter;
        m_captured_piece     = other.m_captured_piece;
        m_history            = other.m_history;
        m_irreversible_index = other.m_irreversible_index;

        for(int i = 0; i < 2; i++)
            memset(m_bitboards[i], 0, sizeof(m_bitboards[i]));
        
        m_castling_rights    = other.m_castling_rights;
        m_history            = other.m_history;
    
        return *this;
    }

    /**
     * @brief Initialize the board with the default chess pieces
     */
    Board& Board::init()
    {
        loadFen(START_FEN);
        return *this;
    }

    /**
     * @brief Update the bitboards, based on the board array
     */
    void Board::updateBitboards()
    {
        for (int i = 0; i < 2; i++)
            memset(m_bitboards[i], 0, sizeof(m_bitboards[i]));
        

        for (int i = 0; i < 64; i++)
        {
            int piece = board[i];
            if (piece == Piece::Empty)
                continue;
            
            bool color = Piece::getColor(piece) == Piece::White;
            int type   = Piece::getType(piece);
            m_bitboards[color][type - 1] |= 1ULL << i;
        }
    }

    /**
     * @brief Load a fen string into the board
     * 
     * @param fen The fen string to load
     */
    void Board::loadFen(std::string fen)
    {
        memset(board, Piece::Empty, sizeof(board));
        std::stringstream ss(fen);
        
        std::unordered_map<char, int> mapping({
            {'p', Piece::Pawn},
            {'r', Piece::Rook},
            {'n', Piece::Knight},
            {'b', Piece::Bishop},
            {'q', Piece::Queen},
            {'k', Piece::King},            
        });

        std::string piece_placement;
        ss >> piece_placement;

        // Read piece placement
        char c = 0;
        size_t i = 0, pos = 0;
        while(c != ' ' && i < piece_placement.size())
        {
            // Read the character
            c = piece_placement[i++];
            char lower = tolower(c);
            // if that's a digit, skip the next n squares,
            // since that's the number of empty squares
            if (isdigit(c)){
                pos += c - '0';   
            }
            // If the character is a piece
            else if(mapping.find(lower) != mapping.end())
            {
                // Set the piece and color
                int piece = mapping[lower];
                // Set the color, if the character is lower case, that's black
                // otherwise, that's white
                if(c == lower){
                    piece |= Piece::Black;
                } else {
                    piece |= Piece::White;
                }
                board[pos++] = piece;
            } 
            // Skip / that's the end of the row
            else if(c == '/'){
                continue;
            }
            // Invalid character
            else {
                return;
            }
        }

        // Set the state of the board
        std::string s_side;
        ss >> s_side;
        m_side = s_side == "w" ? Piece::White : Piece::Black;

        
        // Read castling rights
        i = 0;
        std::string castling_rights;
        ss >> castling_rights;
        CastlingRights cr_obj;

        std::unordered_map<int, int> castling = {
            {'k', CastlingRights::BLACK_KING},
            {'q', CastlingRights::BLACK_QUEEN},
            {'K', CastlingRights::WHITE_KING},
            {'Q', CastlingRights::WHITE_QUEEN},
        };

        while(c != ' ' && i < castling_rights.size())
        {
            c = castling_rights[i++];
            if (c == '-')
            { // no castling for both sides
                i++;
                cr_obj = CastlingRights::NONE;
                break;
            }

            if(castling.find(c) == castling.end())
                return;
            
            cr_obj.add(castling[c]);
        }
        // Set the castling rights
        m_castling_rights = cr_obj;

        // Read enpassant target square
        std::string s_target;
        ss >> s_target;
        int enpassant = str_to_square(s_target);
        m_enpassant_target = enpassant == -1 ? 0 : enpassant;
        
        // Read halfmove clock
        std::string hf;
        ss >> hf;
        m_halfmove_clock = std::stoi(hf);

        // Read fullmove counter
        std::string full_move;
        ss >> full_move;
        m_fullmove_counter = std::stoi(full_move);

        m_captured_piece = Piece::Empty;

        // Update bitboards
        updateBitboards();
        (void)hash();
        verify_castling_rights();

        // Read the 'moves' part
        std::string moves;
        if (ss >> moves && moves == "moves")
        {
            // Read the moves
            while(ss >> moves)
            {
                // Convert the move to the move object
                Move move = match(Move::fromUci(moves));

                if (!isLegal(move))
                    break;

                makeMove(move);
            }
        }
    }

    /**
     * @brief Get the fen string of the current board
     */
    std::string Board::getFen()
    {
        std::string fen;
        fen.reserve(128);

        std::unordered_map<int, char> mapping({
            {Piece::Pawn, 'p'},
            {Piece::Rook, 'r'},
            {Piece::Knight, 'n'},
            {Piece::Bishop, 'b'},
            {Piece::Queen, 'q'},
            {Piece::King, 'k'},            
        });

        int pos = 0;

        // Write piece placement
        for(int i = 0; i < 64; i++)
        {
            if(i % 8 == 0 && i != 0){
                if(pos != 0){
                    fen += '0' + pos;
                    pos = 0;
                }
                fen += '/';
            }
            if(board[i] == Piece::Empty){
                pos++;
                continue;
            }

            if (pos != 0){
                fen += '0' + pos;
                pos = 0;
            }

            int p = board[i];
            char c = mapping[Piece::getType(p)];
            c = Piece::getColor(p) == Piece::Black ? c : toupper(c);
            fen += c;
        }
        if(pos != 0){
            fen += '0' + pos;
        }

        // write the m_side
        fen += ' ';
        fen += m_side == Piece::Black ? 'b' : 'w';


        std::pair<char, int> castling_rights[4] = {
            {'K', CastlingRights::WHITE_KING},
            {'Q', CastlingRights::WHITE_QUEEN},
            {'k', CastlingRights::BLACK_KING},
            {'q', CastlingRights::BLACK_QUEEN},
        };

        // write castling rights
        fen += ' ';
        if (m_castling_rights.none()){
            fen += '-';
        } 
        else {
            for (int i = 0; i < 4; i++){
                if(m_castling_rights.has(castling_rights[i].second)){
                    fen += castling_rights[i].first;
                }
            }
        }

        // write enpassant target square
        fen += ' ';
        if (m_enpassant_target == 0){
            fen += '-';
        } 
        else {
            fen += square_to_str(m_enpassant_target);
        }

        // write halfmove clock
        fen += ' ';
        fen += std::to_string(m_halfmove_clock);

        // write fullmove counter
        fen += ' ';
        fen += std::to_string(m_fullmove_counter);

        return fen;
    }

    /**
     * @brief Match given move (with no flags) with the legal moves 
     * (by comparing the 'from', 'to' parts).
     * This is usefull if you don't know the flags of the move
     * @return Valid move if found in legal moves, otherwise a null move
     */
    Move Board::match(Move move)
    {
        MoveList moves = generateLegalMoves();

        for(size_t i = 0; i < moves.size(); i++)
        {
            if (moves[i].movePart() == move.movePart())
            {
                // If the flags are not empty, return the move
                if (move.getFlags() != Move::FLAG_NONE)
                    return move;
                
                // If the flags are empty, return the move from the legal moves
                return moves[i];
            }
        }
        
        // If the move is not found, return a null move
        return Move();
    }

    /**
     * @brief Print the board, for debugging purposes
     */
    void Board::print()
    {
        std::cout << "  a b c d e f g h\n";
        for(int i = 0; i < 8; i++)
        {
            std::cout << 8 - i << " ";
            for(int j = 0; j < 8; j++)
            {
                int piece = board[i * 8 + j];
                std::cout << Piece::toChar(piece, true) << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";

        for (int color = 0; color < 2; color++)
        {
            for (int type = 0; type < 6; type++)
            {
                std::cout << "Bitboard for " << Piece::toChar(
                    Piece::createPiece(type + 1, color == 1 ? Piece::White : Piece::Black
                ), true) << ":\n";

                Bitboard bb = m_bitboards[color][type];
                for (int i = 0; i < 64; i++)
                {
                    std::cout << (bb & (1ULL << i) ? "1 " : "0 ");
                    if (i % 8 == 7)
                        std::cout << "\n";
                }
                std::cout << "\n";
            }
        }

        std::cout << "Side: " << (m_side == Piece::White ? "White" : "Black") << "\n";
        std::cout << "Enpassant target: " << square_to_str(m_enpassant_target) << "\n";
        std::cout << "Halfmove clock: " << m_halfmove_clock << "\n";
        std::cout << "Fullmove counter: " << m_fullmove_counter << "\n";
        std::cout << "Castling rights: " << m_castling_rights.str() << "\n";
        std::cout << "Captured piece: " << Piece::toChar(m_captured_piece) << "\n";

        MoveList moves = generateLegalCaptures();
        for (auto& move : moves)
        {
            std::cout << Move(move).uci() << "\n";
        }

        std::cout << "Move stack:\n";
        for (auto& state : m_history)
        {
            std::cout << Move(state.move).uci() << "\n";
        }
    }

    bool Board::checkIntegrity()
    {
        // Check if the bitboards are correct (same as the board array)
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                Bitboard bb = m_bitboards[i][j];
                for (int k = 0; k < 64; k++)
                {
                    int piece  = board[k];
                    int type   = Piece::getType(piece);
                    bool color = Piece::getColor(piece) == Piece::White;
                    if (type == j + 1 && color == i)
                    {
                        if (!(bb & (1ULL << k)))
                        {
                            std::cout << "Missing bitboard for " << Piece::toChar(piece) << 
                                " at " << square_to_str(k) << "\n";
                            return false;
                        }
                    }
                    else
                    {
                        if (bb & (1ULL << k))
                        {
                            std::cout << "Extra bitboard for " << Piece::toChar(j + 1) << 
                                " at " << square_to_str(k) << "\n";
                            // Print whole bitboard
                            dbitboard(bb);
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    /**
     * @brief Generate hash of the board
     */
    uint64_t Board::hash()
    {
        m_hash = 0;

        for(int type = 0; type < 6; type++){
            uint64_t white = m_bitboards[1][type];
            uint64_t black = m_bitboards[0][type];

            while(white){
                m_hash ^= Zobrist::hash_pieces[0][type][pop_lsb1(white)];
            }
            while(black){
                m_hash ^= Zobrist::hash_pieces[1][type][pop_lsb1(black)];
            }
        }

        if (m_side == Piece::Black)
            m_hash ^= Zobrist::hash_turn;

        int rank = m_enpassant_target >> 3;
        if (m_enpassant_target != 0)
            m_hash ^= Zobrist::hash_enpassant[rank];

        m_hash ^= Zobrist::hash_castling[castlingRights().get()];

        return m_hash;
    }

    /**
     * @brief Push the current state of the board to the history
     */
    void Board::push_state(Move move)
    {
        State state;
        state.hash = hash();
        state.side_to_move = m_side;
        state.captured_piece = m_captured_piece;
        state.castling_rights = m_castling_rights.get();
        state.enpassant_target = m_enpassant_target;
        state.halfmove_clock = m_halfmove_clock;
        state.fullmove_counter = m_fullmove_counter;
        state.move = move;

        m_history.push_back(state);
    }

    /**
     * @brief Check if given move is legal
     */
    bool Board::isLegal(Move move)
    {
        if (!move)
            return false;

        MoveList moves = generateLegalMoves();
        
        for (size_t i = 0; i < moves.size(); i++)
        {
            if (moves[i] == move)
                return true;
        }

        return false;
    }

    /**
     * @brief Check if the board is in a threefold repetition
     */
    bool Board::isRepetition()
    {
        int count = 0;
        for (size_t i = m_irreversible_index; i < m_history.size(); i++)
        {
            if (m_history[i].hash == m_hash)
                count++;
        }
        return count >= 3;
    }

    /**
     * @brief Make a move on the board, updates the board state (board array, bitboards, side to move, etc.)
     * @warning Doesn't check if the move is legal, technically you may make any move
     */
    void Board::makeMove(Move move)
    {
        // Push the current state to the history
        push_state(move);

        Square to           = move.getTo();
        Square from         = move.getFrom();
        int type            = Piece::getType(board[from]);
        bool is_white       = Piece::isWhite(board[from]);
        const int rights[2] = { CastlingRights::BLACK, CastlingRights::WHITE };

        // Update fullmove counter
        if(m_side == Piece::Black){
            m_fullmove_counter++;
        }

        // Update halfmove clock, if the move is a pawn move 
        // or a capture, reset the clock
        if (type == Piece::Pawn){
            m_halfmove_clock = 0;
        } else {
            m_halfmove_clock++;
        }

        // Set as empty, it may change if the move is a capture
        m_captured_piece = Piece::Empty; 

        // If that's a capture, remove the captured piece
        if(move.isCapture())
        {
            // Set the irreversible index
            m_irreversible_index = m_history.size();
            m_halfmove_clock     = 0;
            Square offset        = 0;

            if(move.isEnPassant())
                offset = is_white ? 8 : -8;
            
            // Capture the piece, remove it from the board
            Square captured_pos = to + offset;
            m_captured_piece    = board[captured_pos];
            board[captured_pos] = Piece::Empty;

            // Delete the captured piece from the bitboard
            m_bitboards[!is_white][Piece::getType(m_captured_piece) - 1] &= ~(1ULL << (captured_pos));
        }

        // Reset the enpassant target
        m_enpassant_target = 0;

        if(move.isCastle())
        {
            bool is_king_castle = move.isKingCastle();
            m_irreversible_index = m_history.size();

            const int castling_pos[2][2] = {
                {0, 56}, // queen castle 
                {7, 63} // king castle
            };

            const int rooks_to[2][2] = {
                {3, 59}, // queen castle
                {5, 61} // king castle
            };

            int rook_from = castling_pos[is_king_castle][is_white],
                rook_to   = rooks_to[is_king_castle][is_white];
            
            // Update castling rights
            m_castling_rights.remove(rights[is_white]);

            // Move the rook (king will be handled outside of this block)
            board[rook_to] = board[rook_from];
            board[rook_from] = Piece::Empty;

            // Update the bitboard for the rook
            updateBitboard(is_white, Piece::Rook - 1, rook_from, rook_to);
        }

        // If it's double move, set the enpassant target
        if (move.isDoubleMove())
        {
            const int dir[2] = {-8, 8};
            m_enpassant_target = to + dir[is_white];
            m_irreversible_index = m_history.size();
        }

        // Update the bitboard for the moved piece
        if (move.isPromotion())
        {
            int promo_type = Piece::promotionPieces[move.getPromotionPiece()];
            board[from] = promo_type | m_side;
            m_bitboards[is_white][Piece::Pawn - 1] &= ~(1ULL << from); // remove the pawn
            m_bitboards[is_white][promo_type - 1]  |= 1ULL << from; // add the promoted piece (not moved)
            m_irreversible_index = m_history.size();
            type = promo_type; // update this, since in the end we will update the bitboard, based on the type
        }

        // Update castling rights & king position
        if (type == Piece::King)
        {
            m_castling_rights.remove(rights[is_white]);
        }
        else if (type == Piece::Rook)
        {
            // If the rook is moved from the starting position, remove the castling
            // rights for that side
            const int side_rights[2][2] = {
                {CastlingRights::BLACK_QUEEN, CastlingRights::BLACK_KING}, 
                {CastlingRights::WHITE_QUEEN, CastlingRights::WHITE_KING}
            };

            if (from == 0 || from == 56)
                m_castling_rights.remove(side_rights[is_white][0]);
            else if (from == 7 || from == 63)
                m_castling_rights.remove(side_rights[is_white][1]);
        }

        // Move the piece
        board[to] = board[from];
        board[from] = Piece::Empty;

        // Update the bitboard for the moved piece
        updateBitboard(is_white, type - 1, from, to);
        // m_bitboards[is_white][type - 1] &= ~(1ULL << from);
        // m_bitboards[is_white][type - 1] |= 1ULL << to;

        m_side = Piece::opposite(m_side);

        if (!checkIntegrity())
        {
            std::cout << "(make)Integrity check failed: " << move.uci() << "\n";
            print();
        }
    }

    /**
     * @brief Undo the last move
     * @warning Doesn't check if the move is legal
     */
    void Board::undoMove(Move move)
    {
        // If there's no history, return
        if (m_history.empty())
            return;
        
        // Get the last state
        State history = m_history.back();
        m_history.pop_back();

        Square to           = move.getTo();
        Square from         = move.getFrom();
        int    type         = Piece::getType(board[to]);
        bool   is_white     = Piece::isWhite(board[to]);
        const int colors[2] = { Piece::Black, Piece::White };


        // Helper function to undo the move, moves the piece from 'to' to 'from'
        // Update the bitboard, based on the type, and the color defined above
        auto undo = [this, &type, &is_white](Square from, Square to){
            board[from] = board[to];
            board[to]   = Piece::Empty;
            updateBitboard(is_white, type - 1, to, from);
        };


        // If promo capture, restore the captured piece + the promoted piece
        if (move.isPromotionCapture())
        {
            board[from] = Piece::Pawn | colors[is_white]; // restore the pawn
            board[to]   = m_captured_piece; // restore the captured piece

            // add the captured piece to the bitboard
            m_bitboards[!is_white][Piece::getType(m_captured_piece) - 1] |= 1ULL << to;
            // remove the promoted piece
            m_bitboards[is_white][type - 1] &= ~(1ULL << to); 
            // add the pawn to the bitboard
            m_bitboards[is_white][Piece::Pawn - 1] |= 1ULL << from;
        }
        else if (move.isCapture())
        {
            // restore the captured piece
            int offset = 0;
            if(move.isEnPassant())
                offset = is_white ? 8 : -8;
            
            // Unmake the move, restore previous position of the piece
            undo(from, to);
            // Put the captured piece back
            Square captured_pos = to + offset;
            board[captured_pos] = m_captured_piece;
            m_bitboards[!is_white][Piece::getType(m_captured_piece) - 1] |= 1ULL << captured_pos;
        }
        else if (move.isPromotion())
        {
            board[from] = Piece::Pawn | colors[is_white]; // restore the pawn
            board[to]   = Piece::Empty; // remove the promoted piece
            // Update the bitboards
            m_bitboards[is_white][type - 1]        &= ~(1ULL << to); // remove the promoted piece
            m_bitboards[is_white][Piece::Pawn - 1] |=  1ULL << from; // add the pawn to the bitboard
        }
        // Restore castling rights and rook positions
        else if (move.isCastle())
        {
            const int rooks_from[2][2] = {
                {0, 56}, // queen castle
                {7, 63} // king castle
            };
            const int rooks_to[2][2] = {
                {3, 59}, // queen castle
                {5, 61} // king castle
            };

            // Get rook starting position
            bool is_king_castle = move.isKingCastle();
            Square rook_from    = rooks_from[is_king_castle][is_white];
            Square rook_to      = rooks_to[is_king_castle][is_white];
            board[rook_from]    = board[rook_to]; // move the rook back
            board[rook_to]      = Piece::Empty; // empty the rook's previous position
            updateBitboard(is_white, Piece::Rook - 1, rook_to, rook_from); // update the bitboard
            undo(from, to); // move the king back
        }
        else 
        {
            // Quiet move, just move the piece back
            undo(from, to);
        }

        // Restore other states
        m_hash             = history.hash;
        m_side             = history.side_to_move;
        m_halfmove_clock   = history.halfmove_clock;
        m_enpassant_target = history.enpassant_target;
        m_castling_rights  = history.castling_rights;
        m_fullmove_counter = history.fullmove_counter;
        m_captured_piece   = history.captured_piece;

        if (!checkIntegrity())
        {
            std::cout << "(unmake)Integrity check failed: " << move.uci() << "\n";
            print();
        }
    }

    /**
     * @brief Verify the castling rights, it may delete the castling rights 
     * if the rooks or the king are not in the correct position
     */
    void Board::verify_castling_rights()
    {
        const int castling_data[2][3] = {
            {4, 0, 7}, // black king data: starting position, rook positions
            {60, 56, 63} // white king (same as above)
        };
        const int kings[2] = {
            bitScanForward(m_bitboards[0][Piece::King - 1]),
            bitScanForward(m_bitboards[1][Piece::King - 1])
        };
        const int castling_rights[2][2] = {
            {CastlingRights::BLACK_QUEEN, CastlingRights::BLACK_KING},
            {CastlingRights::WHITE_QUEEN, CastlingRights::WHITE_KING}
        };

        // Check if the rooks are in the correct position
        for (int is_white = 0; is_white < 2; is_white++)
        {
            for(int i = 0; i < 2; i++)
            {
                int rook_target = castling_data[is_white][i + 1];
                if (Piece::getType(board[rook_target]) != Piece::Rook){
                    m_castling_rights.remove(castling_rights[is_white][i]);
                }
            }
        }

        // Check if the king is in the correct position
        for (int is_white = 0; is_white < 2; is_white++)
        {
            if (kings[is_white] != castling_data[is_white][0]){
                m_castling_rights.remove(castling_rights[is_white][0]);
                m_castling_rights.remove(castling_rights[is_white][1]);
            }
        }
    }

    // --------------------- Move generation ---------------------

    /**
     * @brief Get the rook attacks for a given square, attacks are both squares attacked and pieces
     */
    inline uint64_t rookAttacks(uint64_t occupied, int sq)
    {
        auto& rookMagics = MagicBitboards::rookMagics[sq];
        return MagicBitboards::rookAttacks[sq][((occupied & rookMagics.mask) * rookMagics.magic) >> rookMagics.shift];
    }

    /**
     * @brief Get the bishop attacks for a given square
     */
    inline uint64_t bishopAttacks(uint64_t occupied, int sq)
    {
        auto& bishopMagics = MagicBitboards::bishopMagics[sq];
        return MagicBitboards::bishopAttacks[sq][((occupied & bishopMagics.mask) * bishopMagics.magic) >> bishopMagics.shift];
    }

    inline uint64_t queenAttacks(uint64_t occupied, int sq)
    {
        return rookAttacks(occupied, sq) | bishopAttacks(occupied, sq);
    }

    inline uint64_t knightAttacks(uint64_t occupied, int sq)
    {
        return chess::Board::pieceAttacks[chess::Board::KNIGHT_TYPE][sq];
    }

    /**
     * @brief Get the x-ray attacks for a rook
     * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
     */
    inline uint64_t xRayRookAttacks(uint64_t occupied, uint64_t blockers, int rooksq)
    {
        uint64_t attacks = rookAttacks(occupied, rooksq); // get the valid rook attacks (all pieces involved)
        blockers &= attacks; // take only the blockers that are in the attacks (attacked blockers)
        return attacks ^ rookAttacks(occupied ^ blockers, rooksq); // get the x-ray attacks (attacks through the blockers)
    }

    /**
     * @brief Get the x-ray attacks for a bishop
     * @author https://www.chessprogramming.org/X-ray_Attacks_(Bitboards)#ModifyingOccupancy
     */
    inline uint64_t xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int bishopsq)
    {
        uint64_t attacks = bishopAttacks(occupied, bishopsq);
        blockers &= attacks;
        return attacks ^ bishopAttacks(occupied ^ blockers, bishopsq);
    }

    typedef uint64_t(*attacks_func_t)(uint64_t, int);
    
    /**
     * @brief Get the pinner for a given square
     */
    inline int getPinner(uint64_t pinners, int sq, int king)
    {
        Bitboard pinner  = pinners;
        Bitboard bitsq   = 1ULL << sq;
        Square pinner_sq = 0;

        while(pinner){
            pinner_sq = pop_lsb1(pinner);
            if (chess::Board::in_between[pinner_sq][king] & bitsq)
                break;
        }
        return pinner_sq;
    }

    /**
     * @brief Generate sliding moves for a given piece type when king isn't in check
     */
    void _gen_sliding_moves_no_check(
        chess::MoveList* moves, Bitboard piece_bb,
        uint64_t occupied, uint64_t enemy_king, uint64_t enemy_pieces, 
        uint64_t pinned, uint64_t pinners, int king, bool is_white,
        attacks_func_t attackFunc
    )
    {
        Bitboard bitboard = piece_bb;
        Bitboard bmoves, captures;

        while(bitboard)
        {
            int sq = pop_lsb1(bitboard);
            bmoves = attackFunc(occupied, sq);
            captures = bmoves & enemy_pieces; 
            bmoves &= ~occupied;

            // If the piece is pinned, restrict the moves
            if (pinned & (1ULL << sq)){
                // Get the pinning piece
                int pinner_sq = getPinner(pinners, sq, king);
                bmoves &= chess::Board::in_between[pinner_sq][king];
                captures &= (1ULL << pinner_sq);
            }

            while(bmoves) moves->add(Move::fmove(sq, pop_lsb1(bmoves), Move::FLAG_NONE));
            while(captures) moves->add(Move::fmove(sq, pop_lsb1(captures), Move::FLAG_CAPTURE));
        }
    }

    /**
     * @brief Generate moves for pieces (without pawn & king), when king is in check
     */
    void _gen_pieces_moves_in_check(
        chess::MoveList* moves, Bitboard piece_bb,
        uint64_t occupied, uint64_t attackers, uint64_t not_pinned, 
        uint64_t block_path, uint64_t enemy_king, int attackers_sq, 
        bool is_white, attacks_func_t attackFunc
    )
    {
        Square   sq;
        Bitboard block_moves;
        Bitboard captures;
        Bitboard bitboard = piece_bb & not_pinned;

        while(bitboard)
        {
            sq           = pop_lsb1(bitboard);
            block_moves  = attackFunc(occupied, sq);
            captures     = block_moves & attackers;
            block_moves &= ~occupied & block_path;
            // If that's a capture, add the move
            if (captures){
                moves->add(Move::fmove(sq, attackers_sq, Move::FLAG_CAPTURE));
            }

            while(block_moves){
                moves->add(Move::fmove(sq, pop_lsb1(block_moves), Move::FLAG_NONE));
            }
        }
    }

    /**
     * @brief Generate moves and apply the filter
     */
    MoveList Board::filterMoves(MoveFilter filter)
    {
        return generateLegalMoves().filter(filter);
    }

    /**
     * @brief Generate all legal captures for the current board
     */
    MoveList Board::generateLegalCaptures()
    {
        return generateLegalMoves().filter([](Move move){ return move.isCapture(); });
    }

    /**
     * @brief Generate all the moves for the current board
     */
    MoveList Board::generateLegalMoves()
    {
        MoveList moves;

        bool is_white            = turn();
        bool is_enemy            = !is_white;
        Bitboard occupied        = this->occupied();
        Bitboard occupied_noking = occupied ^ m_bitboards[is_white][Piece::King - 1];
        Bitboard enemy_pieces    = this->occupied(is_enemy);
        Bitboard allied_pieces   = occupied ^ enemy_pieces;
        Bitboard enemy_king      = m_bitboards[is_enemy][Piece::King - 1];
        Square king              = bitScanForward(m_bitboards[is_white][Piece::King - 1]);

        // Step 1: Generate attacks for enemy pieces (to check if the king is in check)
        Bitboard bitboard;
        Bitboard danger = 0;

        // Generate attacks for enemy bishops
        bitboard = m_bitboards[is_enemy][Piece::Bishop - 1];
        while(bitboard) danger |= bishopAttacks(occupied_noking, pop_lsb1(bitboard));
        
        // Generate attacks for enemy rooks
        bitboard = m_bitboards[is_enemy][Piece::Rook - 1];
        while (bitboard) danger |= rookAttacks(occupied_noking, pop_lsb1(bitboard));
        
        // Generate attacks for enemy knights
        bitboard = m_bitboards[is_enemy][Piece::Knight - 1];
        while (bitboard) danger |= Board::pieceAttacks[Board::KNIGHT_TYPE][pop_lsb1(bitboard)];

        // Generate attacks for enemy queen(s)
        bitboard = m_bitboards[is_enemy][Piece::Queen - 1];
        while(bitboard)
        {
            int sq = pop_lsb1(bitboard);
            danger |= rookAttacks(occupied_noking, sq) | bishopAttacks(occupied_noking, sq);
        }

        // Generate attacks for enemy pawns
        bitboard = m_bitboards[is_enemy][Piece::Pawn - 1];
        while(bitboard) danger |= Board::pawnAttacks[is_enemy][pop_lsb1(bitboard)];
        
        // Generate attacks for enemy king
        danger |= Board::pieceAttacks[Board::KING_TYPE][bitScanForward(enemy_king)];

        // Step 2: Generate pinned pieces
        // Source (modified): https://www.chessprogramming.org/Pinned_Pieces

        // Pinned pieces
        Bitboard pinned = 0;
        Bitboard pinners = 0;

        // Rook / Queen pinners
        Bitboard pinner = xRayRookAttacks(occupied, allied_pieces, king) & oppRooksQueens(is_white);
        pinners |= pinner;
        while(pinner) pinned |= Board::in_between[pop_lsb1(pinner)][king] & allied_pieces;

        // Bishop / Queen pinners
        pinner = xRayBishopAttacks(occupied, allied_pieces, king) & oppBishopsQueens(is_white);
        pinners |= pinner;
        while(pinner) pinned |= Board::in_between[pop_lsb1(pinner)][king] & allied_pieces;

        m_in_check = false;

        // Step 3: Generate moves for this side
        // Check if the king is in check

        if (danger & m_bitboards[is_white][Piece::King - 1])
        {
            m_in_check = true;
            // Check the number of attackers
            Bitboard attackers = 0;
            attackers |= bishopAttacks(occupied, king) & oppBishopsQueens(is_white);
            attackers |= rookAttacks(occupied, king) & oppRooksQueens(is_white);
            attackers |= Board::pieceAttacks[Board::KNIGHT_TYPE][king] & bitboards(is_enemy)[Piece::Knight - 1];
            attackers |= Board::pawnAttacks[is_white][king] & bitboards(is_enemy)[Piece::Pawn - 1];

            // If there are more than one attackers, the king is in double check, only king moves are allowed
            if (attackers & (attackers - 1)){
                // Generate king moves
                // King can only move to evade the check
                Bitboard kmoves = Board::pieceAttacks[Board::KING_TYPE][king] & ~occupied & ~danger;
                Bitboard captures = Board::pieceAttacks[Board::KING_TYPE][king] & enemy_pieces & ~danger;

                while(kmoves) moves.add(Move::fmove(king, pop_lsb1(kmoves), Move::FLAG_NONE));
                while(captures) moves.add(Move::fmove(king, pop_lsb1(captures), Move::FLAG_CAPTURE));
                
                return moves;
            }

             // Generate moves to block the check,
            // The things to look out for when generating moves:
            // - My pieces might be pinned:
            //   - pinned pieces cannot move when the king is in check (them moving would expose the king)
            // - Pieces can only either capture the attacker or block the path
            // - Enpassant is possible only if the attacker is a pawn
            // - King may only move to evade the check
            int attackers_sq = bitScanForward(attackers);
            Bitboard block_path = Board::in_between[attackers_sq][king];

            // Generate moves for not pinned pieces
            Bitboard not_pinned = ~pinned & allied_pieces;

            // Generate moves for bishops
            _gen_pieces_moves_in_check(
                &moves, m_bitboards[is_white][BISHOP_TYPE], occupied, attackers, 
                not_pinned, block_path, enemy_king, attackers_sq, 
                is_white, bishopAttacks
            );

            // Generate moves for rooks
            _gen_pieces_moves_in_check(
                &moves, m_bitboards[is_white][ROOK_TYPE], occupied, attackers, 
                not_pinned, block_path, enemy_king, attackers_sq, 
                is_white, rookAttacks
            );

            // Generate moves for queens
            _gen_pieces_moves_in_check(
                &moves, m_bitboards[is_white][QUEEN_TYPE], occupied, attackers, 
                not_pinned, block_path, enemy_king, attackers_sq, 
                is_white, queenAttacks
            );

            // Generate moves for knights
            _gen_pieces_moves_in_check(
                &moves, m_bitboards[is_white][KNIGHT_TYPE], occupied, attackers, 
                not_pinned, block_path, enemy_king, attackers_sq, 
                is_white, knightAttacks
            );

            // Generate moves for pawns
            // Enpassant is possible only if the attacker is a pawn
            bitboard = m_bitboards[is_white][PAWN_TYPE] & not_pinned;
            const int offset[2] = {-8, 8};
            const int ranks[2] = {1, 6};

            while(bitboard)
            {
                int sq = pop_lsb1(bitboard);
                Bitboard captures = Board::pawnAttacks[is_white][sq] & attackers;
                Bitboard enpassant = m_enpassant_target ? Board::pawnAttacks[is_white][sq] & (1ULL << (m_enpassant_target)) : 0;
                int rank = sq >> 3;

                // If the enpassant is possible and the attacker is on a valid capture square, then add the move
                if (enpassant && attackers & (1ULL << (m_enpassant_target + offset[is_white]))){
                    moves.add(Move::fmove(sq, m_enpassant_target, Move::FLAG_ENPASSANT_CAPTURE));
                }
                // If that's a capture, add the move
                if (captures){
                    // Generate captures promoting moves (the pawn is on the either 2nd or 7th rank)
                    if (rank == ranks[is_enemy]){
                        moves.add(Move::fmove(sq, attackers_sq, Move::FLAG_ROOK_PROMOTION_CAPTURE));
                        moves.add(Move::fmove(sq, attackers_sq, Move::FLAG_BISHOP_PROMOTION_CAPTURE));
                        moves.add(Move::fmove(sq, attackers_sq, Move::FLAG_KNIGHT_PROMOTION_CAPTURE));
                        moves.add(Move::fmove(sq, attackers_sq, Move::FLAG_QUEEN_PROMOTION_CAPTURE));
                    } else {
                        moves.add(Move::fmove(sq, attackers_sq, Move::FLAG_CAPTURE));
                    }
                }

                // Generate normal pawn moves
                int n = Board::mailbox[Board::mailbox64[sq] + Board::pawn_move_offsets[is_white][0]];
                Bitboard pmoves = (1ULL << n) & ~occupied;

                // If the pawn push is blocked, stop here
                if(!pmoves)
                    continue;

                if (pmoves & block_path){
                    // If the pawn is on the 2nd (black) or 7th rank (white), generate promotion moves
                    if (rank == ranks[!is_white]){
                        int to = bitScanForward(pmoves);
                        moves.add(Move::fmove(sq, to, Move::FLAG_ROOK_PROMOTION));
                        moves.add(Move::fmove(sq, to, Move::FLAG_BISHOP_PROMOTION));
                        moves.add(Move::fmove(sq, to, Move::FLAG_KNIGHT_PROMOTION));
                        moves.add(Move::fmove(sq, to, Move::FLAG_QUEEN_PROMOTION));
                        continue;
                    }      
                    moves.add(Move::fmove(sq, n, Move::FLAG_NONE));
                }

                // If the pawn is on the 2nd (white) or 7th rank (black), generate double pawn push
                if (rank == ranks[is_white]){
                    n = Board::mailbox[Board::mailbox64[sq] + Board::pawn_move_offsets[is_white][1]];
                    if ((1ULL << n) & ~occupied & block_path){
                        moves.add(Move::fmove(sq, n, Move::FLAG_DOUBLE_PAWN));
                    }
                }
            }

            // Return the moves after generation
            return moves;
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

        // Generate moves for bhisops
        _gen_sliding_moves_no_check(
            &moves, m_bitboards[is_white][BISHOP_TYPE], 
            occupied, enemy_king, enemy_pieces,
            pinned, pinners, king, is_white, 
            bishopAttacks
        );

        // Generate moves for rooks
        _gen_sliding_moves_no_check(
            &moves, m_bitboards[is_white][ROOK_TYPE], 
            occupied, enemy_king, enemy_pieces,
            pinned, pinners, king, is_white, 
            rookAttacks
        );

        // Generate moves for queens
        _gen_sliding_moves_no_check(
            &moves, m_bitboards[is_white][QUEEN_TYPE], 
            occupied, enemy_king, enemy_pieces,
            pinned, pinners, king, is_white, 
            queenAttacks
        );

        // Generate moves for knights
        bitboard = bitboards(is_white)[Piece::Knight - 1];
        while(bitboard){
            int sq = pop_lsb1(bitboard);
            bmoves = Board::Board::pieceAttacks[Board::KNIGHT_TYPE][sq];

            // If knight is pinned, it cannot move
            if (pinned & (1ULL << sq)){
                continue;
            }

            captures = bmoves & enemy_pieces;
            bmoves &= ~occupied;
            while(bmoves) moves.add(Move::fmove(sq, pop_lsb1(bmoves), Move::FLAG_NONE));
            while(captures) moves.add(Move::fmove(sq, pop_lsb1(captures), Move::FLAG_CAPTURE));
        }

        // Generate moves for pawns
        bitboard = m_bitboards[is_white][PAWN_TYPE];
        const int offset[2] = {8, -8};
        const int ranks[2] = {1, 6}; // board is inversed, so the ranks are different
        while(bitboard)
        {
            int sq = pop_lsb1(bitboard);
            uint64_t enpassant_target = m_enpassant_target ? 1ULL << m_enpassant_target : 0;
            uint64_t pinline = 0;
            captures = Board::pawnAttacks[is_white][sq] & enemy_pieces;
            int rank = sq >> 3;

            // If the piece is pinned, restrict the moves
            if (pinned & (1ULL << sq)){
                // Get the pinning piece & update the pin line
                int pinner_sq = getPinner(pinners, sq, king);
                pinline = Board::in_between[pinner_sq][king];
                captures &= (1ULL << pinner_sq);
                enpassant_target &= pinline;
            }

            // Generate captures promoting moves (the pawn is on the either 2nd or 7th rank)
            if (rank == ranks[is_enemy]){
                while(captures) {
                    int cap_sq = pop_lsb1(captures);
                    moves.add(Move::fmove(sq, cap_sq, Move::FLAG_ROOK_PROMOTION_CAPTURE));
                    moves.add(Move::fmove(sq, cap_sq, Move::FLAG_BISHOP_PROMOTION_CAPTURE));
                    moves.add(Move::fmove(sq, cap_sq, Move::FLAG_KNIGHT_PROMOTION_CAPTURE));
                    moves.add(Move::fmove(sq, cap_sq, Move::FLAG_QUEEN_PROMOTION_CAPTURE));
                }
            } else {
                while(captures) moves.add(Move::fmove(sq, pop_lsb1(captures), Move::FLAG_CAPTURE));
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
                Bitboard occ = occupied ^ (1ULL << (m_enpassant_target - offset[is_white]));
                Bitboard opRQ = oppRooksQueens(is_white);
                Bitboard ppiner = xRayRookAttacks(occ, allied_pieces, king) & opRQ;
                Bitboard ppinned = 0;
                while(ppiner){
                    ppinned |= Board::in_between[pop_lsb1(ppiner)][king] & allied_pieces;
                }
                
                // Check again if the pawn is pinned
                if (ppinned & (1ULL << sq)){
                    enpassant_target &= pinline;
                }

                // If the pawn is not pinned or it can move along the pin line, add the move
                if (enpassant_target){
                    moves.add(Move::fmove(sq, m_enpassant_target, Move::FLAG_ENPASSANT_CAPTURE));
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
                if (rank == ranks[!is_white])
                {
                    int to = bitScanForward(bmoves);
                    moves.add(Move::fmove(sq, to, Move::FLAG_ROOK_PROMOTION));
                    moves.add(Move::fmove(sq, to, Move::FLAG_BISHOP_PROMOTION));
                    moves.add(Move::fmove(sq, to, Move::FLAG_KNIGHT_PROMOTION));
                    moves.add(Move::fmove(sq, to, Move::FLAG_QUEEN_PROMOTION));
                    continue;
                }
                moves.add(Move::fmove(sq, n, Move::FLAG_NONE));
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
                    moves.add(Move::fmove(sq, n, Move::FLAG_DOUBLE_PAWN));
                }
            }
        }

        // Generate moves for the king
        bmoves = Board::pieceAttacks[Board::KING_TYPE][king] & ~occupied & ~danger;
        captures = Board::pieceAttacks[Board::KING_TYPE][king] & enemy_pieces & ~danger;

        while(bmoves) moves.add(Move::fmove(king, pop_lsb1(bmoves), Move::FLAG_NONE));
        while(captures) moves.add(Move::fmove(king, pop_lsb1(captures), Move::FLAG_CAPTURE));

        // Generate castling moves, I assume that castling rights are correct
        // Hence I always verify the castling rights
        // verify_castling_rights();
        const uint32_t colors[2] = { CastlingRights::BLACK, CastlingRights::WHITE };

        if (castlingRights().has(colors[is_white]))
        {
            bool king_side             = (castlingRights().getKing() & colors[is_white]) != 0;
            bool queen_side            = (castlingRights().getQueen() & colors[is_white]) != 0;
            Bitboard queen_path        = 0b00001100;
            Bitboard king_path         = 0b01100000;
            Bitboard queen_path_no_occ = 0b00001110;

            if(is_white)
            {
                queen_path <<= 56;
                king_path <<= 56;
                queen_path_no_occ <<= 56;
            }

            // King can castle safely only if the square between target position and starting position
            // aren't occupied and aren't attacked
            if (king_side && (king_path & (~occupied) & (~danger)) == king_path){
                moves.add(Move::fmove(king, king + 2, Move::FLAG_KING_CASTLE));
            }

            // Additionally space between rook and king should be empty (in king's side case `king_path` = `king_path_no_occ`)
            if (queen_side && (queen_path & (~occupied) & (~danger)) == queen_path && (queen_path_no_occ & (~occupied)) == queen_path_no_occ){
                moves.add(Move::fmove(king, king - 2, Move::FLAG_QUEEN_CASTLE));
            }
        }

        return moves;
    }

} // namespace chess

