#include <cengine/eval.h>


namespace chess
{
    // Source: https://www.chessprogramming.org/Simplified_Evaluation_Function
    const int piece_values[6] = {100, 320, 20000, 330, 500, 900};
    int white_piece_square_table[6][64] = {
        // for white
        // pawn
        {0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0,},
        // knight
        {-50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,},
        // king empty here, because more defined version is used
        {0},
        // bishop
        {-20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,},
        // rook
        {0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0},
        // queen
        {-20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20},
    };
    int white_king_square_tables[2][64] = {
        // for white
        // middle game
        {-30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20},
        // end game
        {-50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50,}
    };
    static int piece_square_table[2][6][64] = {0};
    static int king_square_tables[2][2][64] = {0};
    static TTable<int> pawn_table;

    /**
     * @brief Initialize the boards for evaluation
     */
    void init_eval()
    {
        // Initialize the pieces tables
        for(int i = 0; i < 6; i++){
            if (i == Piece::King - 1){
                continue;
            }
            for(int j = 0; j < 64; j++){
                piece_square_table[1][i][j] = white_piece_square_table[i][j];
                piece_square_table[0][i][j] = white_piece_square_table[i][63 - j];
            }
        }

        // Initialize the king square tables
        for(int i = 0; i < 2; i++){
            for(int j = 0; j < 64; j++){
                king_square_tables[1][i][j] = white_king_square_tables[i][j];
                king_square_tables[0][i][j] = white_king_square_tables[i][63 - j];
            }
        }
    }

    
    /**
     * @brief Evaluation function for the board in centipawns
     * positive values are good for white, negative values are good for black
     */
    int evaluate(Board* board, CacheMoveGen* c, MoveList* ml)
    {
        int eval = 0;
        bool is_white = board->getSide() == Piece::White;
        bool is_enemy = !is_white;
        int king_sq = bitScanForward(board->bitboards(is_white)[Piece::King - 1]);
        int eking_sq = bitScanForward(board->bitboards(is_enemy)[Piece::King - 1]);

        const uint64_t file_bitboards[8] = {
            0x0101010101010101ULL,
            0x0202020202020202ULL,
            0x0404040404040404ULL,
            0x0808080808080808ULL,
            0x1010101010101010ULL,
            0x2020202020202020ULL,
            0x4040404040404040ULL,
            0x8080808080808080ULL
        };

        // Count the material & piece square tables
        for (int type = 0; type < 6; type++){
            if (type == Piece::King - 1){
                continue;
            }

            uint64_t epieces = board->bitboards(is_enemy)[type];
            uint64_t apieces = board->bitboards(is_white)[type];

            while(epieces){
                int sq = pop_lsb1(epieces);
                eval -= piece_values[type];
                eval -= piece_square_table[is_enemy][type][sq];
            }
            while(apieces){
                int sq = pop_lsb1(apieces);
                eval += piece_values[type];
                eval += piece_square_table[is_white][type][sq];
            }
        }
        
        // Bonus for having the bishop pair
        if (pop_count(board->bitboards(is_white)[Piece::Bishop - 1]) >= 2){
            eval += 50;
        }
        if (pop_count(board->bitboards(is_enemy)[Piece::Bishop - 1]) >= 2){
            eval -= 50;
        }

        // Pawn structure
        // Try to get hashed pawn structure (already calculated)
        uint64_t pawn_hash = get_pawn_hash(board);
        if (pawn_table.contains(pawn_hash)){
            eval += pawn_table.get(pawn_hash);
        } else {
            int pawn_eval = 0;

            // Doubled pawns
            uint64_t pawns = board->bitboards(is_white)[Piece::Pawn - 1];
            uint64_t epawns = board->bitboards(is_enemy)[Piece::Pawn - 1];
            for (int i = 0; i < 8; i++){
                uint64_t file = file_bitboards[i];
                if (pawns & file && pop_count(pawns & file) > 1){
                    pawn_eval -= 10;
                }
                if (epawns & file && pop_count(epawns & file) > 1){
                    pawn_eval += 10;
                }
            }

            // Isolated pawns
            for (int i = 0; i < 8; i++){
                uint64_t file = file_bitboards[i];
                if (pawns & file){
                    if (!(pawns & (file >> 1)) && !(pawns & (file << 1))){
                        pawn_eval -= 10;
                    }
                }
                if (epawns & file){
                    if (!(epawns & (file >> 1)) && !(epawns & (file << 1))){
                        pawn_eval += 10;
                    }
                }
            }

            // Connected pawns
            for (int i = 0; i < 8; i++){
                uint64_t file = file_bitboards[i];
                if (pawns & file){
                    if (pawns & (file >> 8) || pawns & (file << 8)){
                        pawn_eval += 10;
                    }
                }
                if (epawns & file){
                    if (epawns & (file >> 8) || epawns & (file << 8)){
                        pawn_eval -= 10;
                    }
                }
            }

            // Store the pawn hash
            pawn_table.store(pawn_hash, pawn_eval);
            eval += pawn_eval;
        }

        // const uint64_t enemy_board_side[2] = {
        //     0xFFFFFFFF00000000ULL, // for black
        //     0x00000000FFFFFFFFULL, // for white
        // };
        // // Mobility / Activity (doesn't work properly -> enemy activity is too high)
        // uint64_t allied_activity = c->activity & enemy_board_side[is_white];
        // uint64_t enemy_activity = c->danger & enemy_board_side[is_enemy];

        // eval += (pop_count(allied_activity) - pop_count(enemy_activity)) * 5;
        

        // King square tables
        bool is_endgame = false;

        if (pop_count(board->pieces()) <= 6 || ((pop_count(board->queens()) == 0) && (pop_count(board->rooks()) == 0))){
            is_endgame = true;
        }

        eval += king_square_tables[is_white][is_endgame][king_sq];
        eval -= king_square_tables[is_enemy][is_endgame][eking_sq];

        return eval;
    }

    /**
     * @brief Get the status of the game (ongoing, checkmate, stalemate, draw)
     */
    GameStatus get_status(Board* b, GameHistory* gh, MoveList* ml)
    {
        if (b->halfmoveClock() >= 100){
            return DRAW;
        }

        // Checkmate / stalemate
        if (ml->size() == 0){
            if (b->inCheck()){
                return CHECKMATE;
            }
            return STALEMATE;
        }

        // Insufficient material
        if ((b->pieces() | b->pawns()) == 0){
            return DRAW;
        }

        if (gh->repetitions(b, gh->back().hash) >= 3){
            return DRAW;
        }

        return ONGOING;
    }

    /**
     * @brief Get the status of the game (ongoing, checkmate, stalemate, draw)
     */
    GameStatus get_status(Board* board, MoveList *ml)
    {
        if (board->halfmoveClock() >= 100){
            return DRAW;
        }

        // Checkmate / stalemate
        if (ml->size() == 0){
            if (board->inCheck()){
                return CHECKMATE;
            }
            return STALEMATE;
        }

        // Insufficient material
        if ((board->pieces() | board->pawns()) == 0){
            return DRAW;
        }

        if (board->isRepetition()){
            return DRAW;
        }

        return ONGOING;
    }

    /**
     * @brief Get the status of the game (ongoing, checkmate, stalemate, draw)
     */
    GameStatus get_status(Board& board)
    {
        MoveList ml = board.generateLegalMoves();
        return get_status(&board, &ml);
    }

    /**
     * @brief Convert the game status to a string
     */
    std::string game_status_to_string(GameStatus status)
    {
        switch (status)
        {
        case ONGOING:
            return "Ongoing";
        case CHECKMATE:
            return "Checkmate";
        case STALEMATE:
            return "Stalemate";
        case DRAW:
            return "Draw";
        default:
            return "Unknown";
        }
    }
}