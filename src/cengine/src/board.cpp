#include <cengine/board.h>


namespace chess
{
    /*
        Using mailbox approach to represent the board
    */

    const int Board::mailbox64[64] = {
        21, 22, 23, 24, 25, 26, 27, 28,
        31, 32, 33, 34, 35, 36, 37, 38,
        41, 42, 43, 44, 45, 46, 47, 48,
        51, 52, 53, 54, 55, 56, 57, 58,
        61, 62, 63, 64, 65, 66, 67, 68,
        71, 72, 73, 74, 75, 76, 77, 78,
        81, 82, 83, 84, 85, 86, 87, 88,
        91, 92, 93, 94, 95, 96, 97, 98,
    };

    const int Board::mailbox[120] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
        -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
        -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
        -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
        -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
        -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
        -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
        -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    // Pawn attack offsets, access by Piece::Color == Piece::White ([0] -> Black, [1] -> White)
    const int Board::pawn_attack_offsets[2][2] = {
        { 11,  9}, // Black 
        {-11, -9}, // White 
    };

    // Pawn move offsets, access by Piece::Color == Piece::White ([0] -> Black, [1] -> White)
    const int Board::pawn_move_offsets[2][2] = {
        { 10,  20}, // Black 
        {-10, -20}, // White 
    };

    // Piece move offsets, access by Piece::Type - 1 (excluding the pawn)
    const int Board::piece_move_offsets[6][8] = {
        { 0,    0,   0,  0, 0,  0,  0,  0}, // Pawn (not used, Piece::Type = 1)
        {-21, -19, -12, -8, 8, 12, 19, 21}, // Knight (type = 2)
        {-11, -10,  -9, -1, 1,  9, 10, 11}, // King (type = 3)
        {-11,  -9,   9, 11, 0,  0,  0,  0}, // Bishop (type = 4)
        {-10,  -1,   1, 10, 0,  0,  0,  0}, // Rook (type = 5)
        {-11, -10,  -9, -1, 1,  9, 10, 11}, // Queen (type = 6)
    };

    // Number of piece rays, access by Piece::Type - 1 (excluding the pawn)
    const int Board::n_piece_rays[6] = {
        0, 8, 8, 4, 4, 8 // Empty (Pawn), Knight, King, Bishop, Rook, Queen
    };

    // Boolean, wheter the piece is sliding, access by Piece::Type - 1 (excluding the pawn)
    const bool Board::is_piece_sliding[6] = {
        false, false, false, true, true, true // Pawn, Knight, King, Bishop, Rook, Queen
    };


    // Helper bitboards
    uint64_t Board::in_between[64][64] = {0};
    uint64_t Board::pawnAttacks[2][64] = {0};
    uint64_t Board::knightAttacks[64] = {0};
    uint64_t Board::kingAttacks[64] = {0};
    uint64_t Board::queenAttacks[64] = {0};

    const char Board::startFen[57] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    /**
     * @brief Initialize the board with the default chess pieces
     */
    Board& Board::init(){     

        m_side = Piece::White;
        m_halfmove_clock = 0;
        m_fullmove_counter = 1;  
        m_enpassant_target = 0;
        m_castling_rights = CastlingRights::ALL;
        m_captured_piece = Piece::Empty;

        board = std::unique_ptr<int[]>(new int[64]());

        for (auto i=0; i < 8; i++){
            board[i + 8] = Piece::createPiece(Piece::Pawn, Piece::Black);
            board[i + 48] = Piece::createPiece(Piece::Pawn, Piece::White); 
        }
        board[0] = Piece::getRook(Piece::Black);
        board[7] = Piece::getRook(Piece::Black);

        board[56] = Piece::getRook(Piece::White);
        board[63] = Piece::getRook(Piece::White);

        board[1] = Piece::Knight | Piece::Black;
        board[6] = Piece::Knight | Piece::Black;

        board[57] = Piece::Knight | Piece::White;
        board[62] = Piece::Knight | Piece::White;

        board[2] = Piece::Bishop | Piece::Black;
        board[5] = Piece::Bishop | Piece::Black;

        board[58] = Piece::Bishop | Piece::White;
        board[61] = Piece::Bishop | Piece::White;

        board[3] = Piece::Queen | Piece::Black;
        board[4] = Piece::getKing(Piece::Black);

        board[59] = Piece::Queen | Piece::White;
        board[60] = Piece::getKing(Piece::White);

        updateBitboards();
        return *this;
    }

    /**
     * @brief Update the bitboards, this is called after initializing the board
     */
    void Board::updateBitboards(){
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < 6; j++){
                m_bitboards[i][j] = 0;
            }
        }
        for (int i = 0; i < 64; i++){
            int piece = board[i];
            if (piece == Piece::Empty){
                continue;
            }
            bool color = Piece::getColor(piece) == Piece::White;
            int type = Piece::getType(piece);
            m_bitboards[color][type - 1] |= 1ULL << i;
        }
    }

    /**
     * @brief Load a fen string into the board
     * 
     * @param fen The fen string to load
     */
    void Board::loadFen(const char* fen){
        board = std::make_unique<int[]>(64);

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
        while(c != ' ' && i < piece_placement.size()){
            c = piece_placement[i++];
            char lower = tolower(c);
            if (isdigit(c)){
                pos += c - '0';   
            }
            else if(mapping.find(lower) != mapping.end()){
                int piece = mapping[lower];
                if(c == lower){
                    piece |= Piece::Black;
                } else {
                    piece |= Piece::White;
                }
                board[pos++] = piece;
            } else if(c == '/'){
                continue;
            } else {
                return;
            }
        }

        // Read m_side
        std::string s_side;
        ss >> s_side;
        m_side = s_side == "w" ? Piece::White : Piece::Black;

        
        // Read castling rights
        i = 0;
        std::string castling_rights;
        ss >> castling_rights;

        std::unordered_map<int, int> castling = {
            {'k', CastlingRights::BLACK_KING},
            {'q', CastlingRights::BLACK_QUEEN},
            {'K', CastlingRights::WHITE_KING},
            {'Q', CastlingRights::WHITE_QUEEN},
        };

        while(c != ' ' && i < castling_rights.size()){
            c = castling_rights[i++];
            if (c == '-'){ // no castling for both sides
                i++;
                m_castling_rights = CastlingRights::NONE;
                break;
            }
            if(castling.find(c) == castling.end()){
                return;
            }
            m_castling_rights.add(castling[c]);
        }

        // Read enpassant target square
        std::string s_target;
        ss >> s_target;
        m_enpassant_target = str_to_square(s_target);
        m_enpassant_target = m_enpassant_target == -1 ? 0 : m_enpassant_target;
        
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
    }

    /**
     * @brief Get the fen string of the current board
     */
    std::string Board::getFen(){
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
        for(int i = 0; i < 64; i++){
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
        } else {
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
        } else {
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
     * @brief Find all the indices of a piece on the board
     * 
     * @param piece The piece to find (with color and special moves)
     * @return vector<int> The indices of the piece
     */
    std::vector<int> Board::findAll(int piece){
        std::vector<int> indices;
        indices.reserve(16);
        for (auto i=0; i < 64; i++){
            if (board[i] == piece){
                indices.push_back(i);
            }
        }
        return indices;
    }
}

