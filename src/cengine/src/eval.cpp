#include <cengine/eval.h>


namespace chess
{
    // Source: https://www.chessprogramming.org/Simplified_Evaluation_Function
    const int piece_values[6] = {100, 320, 20000, 330, 500, 900};
    int white_piece_square_table[6][64] = {
        // for white
        // pawn (using other table)
        {0},
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

    // For the endgame & middle game
    int white_pawn_square_table[2][64] = {
        // for white
        // middle game
        {
            0,  0,  0,  0,  0,  0,  0,  0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 10, 20, 30, 30, 20, 10, 10,
            5,  5, 10, 25, 25, 10,  5,  5,
            0,  0,  0, 20, 20,  0,  0,  0,
            5, -5,-10,  0,  0,-10, -5,  5,
            5, 10, 10,-20,-20, 10, 10,  5,
            0,  0,  0,  0,  0,  0,  0,  0,
        },
        // endgame
        {
            0,  0,  0,  0,  0,  0,  0,  0,
            50, 50, 50, 50, 50, 50, 50, 50,
            40, 40, 40, 40, 40, 40, 40, 40,
            30, 30, 30, 30, 30, 30, 30, 30,
            20, 20, 20, 20, 20, 20, 20, 20,
            10, 10, 10, 10, 10, 10, 10, 10,
             5,  5,  5,  5,  5,  5,  5,  5,
             0,  0,  0,  0,  0,  0,  0,  0,
        },
    };

    static int manhattan_distance[64][64] = {0};
    static int piece_square_table[2][6][64] = {0}; // [color (white = 1) (black = 0) ][piece][square]
    static int king_square_tables[2][2][64] = {0}; // [color (white = 1) (black = 0) ][endgame = 1/middle game = 0][square]
    static int pawn_square_tables[2][2][64] = {0}; // [color (white = 1) (black = 0) ][endgame = 1/middle game = 0][square]
    static TTable<int> pawn_table(8);

    // https://www.chessprogramming.org/Center_Manhattan-Distance
    const uint8_t center_manhattan_distance[64] = { 
        6, 5, 4, 3, 3, 4, 5, 6,
        5, 4, 3, 2, 2, 3, 4, 5,
        4, 3, 2, 1, 1, 2, 3, 4,
        3, 2, 1, 0, 0, 1, 2, 3,
        3, 2, 1, 0, 0, 1, 2, 3,
        4, 3, 2, 1, 1, 2, 3, 4,
        5, 4, 3, 2, 2, 3, 4, 5,
        6, 5, 4, 3, 3, 4, 5, 6
    };

    /**
     * @brief Initialize the boards for evaluation
     */
    void init_eval()
    {
        // Initialize the pieces tables
        for(int i = 1; i < 6; i++){
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

        // Initialize the pawn square tables
        for(int i = 0; i < 2; i++){
            for(int j = 0; j < 64; j++){
                pawn_square_tables[1][i][j] = white_pawn_square_table[i][j];
                pawn_square_tables[0][i][j] = white_pawn_square_table[i][63 - j];
            }
        }

        // Initialize the manhattan distance
        for (int i = 0; i < 64; i++){
            for (int j = 0; j < 64; j++){
                int rank1 = i / 8;
                int file1 = i % 8;
                int rank2 = j / 8;
                int file2 = j % 8;
                manhattan_distance[i][j] = abs(rank1 - rank2) + abs(file1 - file2);
            }
        }
    }


    /**
     * @brief Check if the game is in the endgame phase, based on the material
     * @param b Board to check
     * @param material_allies Material of the allies without pawns & king
     * @param material_enemy Material of the enemy without pawns & king
     */
    bool check_is_endgame(Board* b, int material_allies, int material_enemy)
    {
        int material = std::min(material_allies, material_enemy);
        uint64_t n_pieces = pop_count(b->pieces());
        uint64_t n_queens = pop_count(b->queens());
        uint64_t n_rooks = pop_count(b->rooks());

        return (
            material <= 1000 || // Less than 1000 centipawns (without pawns)
            // No queens, up to 1 rook per side, and total of 6 pieces (ex. R N B vs r n n)
            (n_pieces <= 4) || // 4 pieces or less (Q, R vs q, r, etc.)
            (n_pieces <= 6 && ((n_queens == 0 && n_rooks <= 2)))
            // Up to 6 pieces, no queens, up to 2 rooks (R, b b vs r, b, n)
        );
    }
    
    /**
     * @brief Evaluation function for the board in centipawns
     * positive values are good for white, negative values are good for black
     */
    int evaluate(Board* board)
    {
        int eval = 0;
        bool is_white = board->getSide() == Piece::White;
        bool is_enemy = !is_white;
        int king_sq = bitScanForward(board->bitboards(is_white)[Piece::King - 1]);
        int eking_sq = bitScanForward(board->bitboards(is_enemy)[Piece::King - 1]);
        int material = 0;
        int ematerial = 0;

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

        // Count the material and piece square table
        for (int type = Piece::Knight - 1; type < 6; type++){
            if (type == Piece::King - 1){
                continue;
            }
            uint64_t bitboard = board->bitboards(is_white)[type];
            while(bitboard){
                material += piece_values[type];
                eval += piece_square_table[is_white][type][pop_lsb1(bitboard)];
            }

            bitboard = board->bitboards(is_enemy)[type];
            while(bitboard){
                ematerial += piece_values[type];
                eval -= piece_square_table[is_enemy][type][pop_lsb1(bitboard)];
            }
        }
        eval += material - ematerial;
        bool is_endgame = check_is_endgame(board, material, ematerial);
        
        // Bonus for having the bishop pair
        if (pop_count(board->bitboards(is_white)[Piece::Bishop - 1]) >= 2){
            eval += 50;
        }
        if (pop_count(board->bitboards(is_enemy)[Piece::Bishop - 1]) >= 2){
            eval -= 50;
        }

        // Pawn structure
        // Try to get hashed pawn structure (already calculated)
        uint64_t pawn_hash = Zobrist::get_pawn_hash(board);
        if (pawn_table.contains(pawn_hash)){
            eval += pawn_table.get(pawn_hash);
        } else {
            int pawn_eval = 0;

            uint64_t pawns = board->bitboards(is_white)[Piece::Pawn - 1];
            uint64_t epawns = board->bitboards(is_enemy)[Piece::Pawn - 1];
            uint64_t bitboard;

            // Pawn square table & material
            bitboard = pawns;
            while (bitboard){
                pawn_eval += pawn_square_tables[is_white][is_endgame][pop_lsb1(bitboard)];
                pawn_eval += piece_values[Piece::Pawn - 1];
            }

            bitboard = epawns;
            while (bitboard){
                pawn_eval -= pawn_square_tables[is_enemy][is_endgame][pop_lsb1(bitboard)];
                pawn_eval -= piece_values[Piece::Pawn - 1];
            }

            // Doubled pawns
            for (int i = 0; i < 8; i++){
                uint64_t file = file_bitboards[i];
                if (pawns & file && pop_count(pawns & file) > 1){
                    pawn_eval -= 10;
                }
                if (epawns & file && pop_count(epawns & file) > 1){
                    pawn_eval += 10;
                }
            }
            
            for (int i = 0; i < 8; i++){
                uint64_t file = file_bitboards[i];

                // Isolated pawns
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

                // Connected pawns
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

                // Passed pawns
                if (pawns & file){
                    if (!(epawns & file) && !(epawns & file >> 1) && !(epawns & file << 1)){
                        pawn_eval += 20;
                    }
                }
                if (epawns & file){
                    // Black passed pawn
                    if (!(pawns & file) && !(pawns & file >> 1) && !(pawns & file << 1)){
                        pawn_eval -= 20;
                    }
                }
            }

            // Store the pawn hash
            pawn_table.store(pawn_hash, pawn_eval);
            eval += pawn_eval;
        }

        // If one side has a significant advantage + the other one has no pawns, king should be more aggressive
        // and drive the other one to a corner, so it's easier to checkmate
        int whotomove = board->getSide() == Piece::White ? 1 : -1;
        int pos_eval = 0;
        if (is_endgame && pop_count(board->bitboards(is_enemy)[Board::PAWN_TYPE]) < 2 && eval * whotomove >= 500){
            // Favor enemy king to be in the corner
            pos_eval += center_manhattan_distance[eking_sq] * 4 * whotomove;
            // Favor own king to be close to the enemy king
            pos_eval += manhattan_distance[king_sq][eking_sq] * 2 * whotomove;
        }
        eval += pos_eval;

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
}