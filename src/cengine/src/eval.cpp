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
            0,     0,   0,   0,   0,   0,   0,   0,
            120, 110,  90,  80,  80,  90, 110, 120,
            94,   90,  70,  50,  50,  70,  90,  94,
            32,   24,  13,   5,  -2,   4,  17,  17,
            13,    9,  -3,  -7,  -7,  -8,   3,  -1,
             4,    7,  -6,   1,   0,  -5,  -1,  -8,
            13,    8,   8,   10, 13,   0,   2,  -7,
             0,    0,   0,    0,  0,   0,  0,    0,
        },
    };

    static int manhattan_distance[64][64] = {0};
    static int piece_square_table[2][6][64] = {0}; // [color (white = 1) (black = 0) ][piece][square]
    static int king_square_tables[2][2][64] = {0}; // [color (white = 1) (black = 0) ][endgame = 1/middle game = 0][square]
    static int pawn_square_tables[2][2][64] = {0}; // [color (white = 1) (black = 0) ][endgame = 1/middle game = 0][square]    
    static int piece_square_tables[2][2][6][64] = {0}; // [color (white = 1) (black = 0) ][endgame = 1/middle game = 0][piece][square]
    static const int endgame_phase_material[6] = {0, 1, 0, 1, 4, 4};
    static int total_phase_material = 0;

    struct __attribute__((packed)) PawnEval {
        int eval: 16;
        int material: 16;
        int endgame_eval: 16;
        int reserved: 16;
    };

    static TTable<PawnEval> pawn_table(16);

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
        pawn_table.getTable().max_load_factor(0.75f);

        for(int i = 0; i < 6; i++){
            total_phase_material += endgame_phase_material[i];
        }
        total_phase_material *= 2;

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

    inline PawnEval eval_pawns(uint64_t pawns, uint64_t epawns, uint64_t file)
    {
        PawnEval eval = {0, 0, 0, 0};
        if (!(pawns & file))
            return eval;
        
        // Doubled pawns
        if (pop_count(pawns & file) > 1)
            eval.eval -= 10;

        // Isolated pawns
        if (!(pawns & (file >> 1)) && !(pawns & (file << 1)))
            eval.eval -= 10;
        
        // Connected pawns
        if (pawns & (file >> 8) || pawns & (file << 8)){
            eval.eval += 10;
            eval.endgame_eval += 15;
        }

        // Passed pawns
        if (!(epawns & file) && !(epawns & (file << 1)) && !(epawns & (file >> 1)))
            eval.endgame_eval += 10;
        
        return eval;
    }
    
    /**
     * @brief Evaluation function for the board in centipawns
     * positive values are good for white, negative values are good for black
     */
    int evaluate(Board* board)
    {
        int eval = 0; // middle game evaluation
        int endgame_eval = 0; // endgame evaluation
        int phase = 0; // phase of the game (0 -> endgame, total_phase_material -> middle game)
        int material = 0; // material evaluation
        bool is_white = board->wside();
        bool is_enemy = !is_white;
        int king_sqrs[2] = {
            bitScanForward(board->bitboards(false)[Piece::King - 1]),
            bitScanForward(board->bitboards(true)[Piece::King - 1])
        };
        int king_sq = king_sqrs[is_white];
        int eking_sq = king_sqrs[is_enemy];

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
                phase += endgame_phase_material[type];
                material += piece_values[type];
                eval += piece_square_table[is_white][type][pop_lsb1(bitboard)];
            }

            bitboard = board->bitboards(is_enemy)[type];
            while(bitboard){
                phase += endgame_phase_material[type];
                material -= piece_values[type];
                eval -= piece_square_table[is_enemy][type][pop_lsb1(bitboard)];
            }
        }
        
        // Bonus for having the bishop pair
        if (pop_count(board->bitboards(is_white)[Piece::Bishop - 1]) >= 2){
            eval += 50;
        }
        if (pop_count(board->bitboards(is_enemy)[Piece::Bishop - 1]) >= 2){
            eval -= 50;
        }
        endgame_eval = eval; // Since i'm not using the endgame square tables for pieces, just copy the middle game one.
        phase = std::min(phase, total_phase_material); // Clamp the phase to the total phase material

        // Pawn structure
        // Try to get hashed pawn structure (already calculated)
        uint64_t pawn_hash = Zobrist::get_pawn_hash(board);
        if (pawn_table.contains(pawn_hash)){
            auto pawn_eval = pawn_table.get(pawn_hash);
            eval += pawn_eval.eval;
            material += pawn_eval.material;
            endgame_eval += pawn_eval.endgame_eval;
        } else {
            PawnEval pawn_eval = {0, 0, 0, 0};

            uint64_t pawns = board->bitboards(is_white)[Piece::Pawn - 1];
            uint64_t epawns = board->bitboards(is_enemy)[Piece::Pawn - 1];
            uint64_t bitboard;

            // Pawn square table & material
            bitboard = pawns;
            while (bitboard){
                int sq = pop_lsb1(bitboard);
                pawn_eval.material += piece_values[Piece::Pawn - 1];
                pawn_eval.eval += pawn_square_tables[is_white][0][sq];
                pawn_eval.endgame_eval += pawn_square_tables[is_white][1][sq];
            }

            bitboard = epawns;
            while (bitboard){
                int sq = pop_lsb1(bitboard);
                pawn_eval.material -= piece_values[Piece::Pawn - 1];
                pawn_eval.eval -= pawn_square_tables[is_enemy][0][sq];
                pawn_eval.endgame_eval -= pawn_square_tables[is_enemy][1][sq];
            }

            PawnEval file_pawns = {0, 0, 0, 0};
            PawnEval file_epawns = {0, 0, 0, 0};
            for (int i = 0; i < 8; i++){
                file_pawns = eval_pawns(pawns, epawns, file_bitboards[i]);
                file_epawns = eval_pawns(epawns, pawns, file_bitboards[i]);

                pawn_eval.eval += file_pawns.eval;
                pawn_eval.endgame_eval += file_pawns.endgame_eval;

                pawn_eval.eval -= file_epawns.eval;
                pawn_eval.endgame_eval -= file_epawns.endgame_eval;
            }

            // Store the pawn hash
            pawn_table.store(pawn_hash, pawn_eval);
            eval += pawn_eval.eval;
            material += pawn_eval.material;
            endgame_eval += pawn_eval.endgame_eval;
        }

        // Add the material evaluation
        eval += material;
        endgame_eval += material;

        // King square table
        eval += king_square_tables[is_white][0][king_sq];
        eval -= king_square_tables[is_enemy][0][eking_sq];
        endgame_eval += king_square_tables[is_white][1][king_sq];
        endgame_eval -= king_square_tables[is_enemy][1][eking_sq];

        // If one side has a significant advantage + the other one has no pawns, king should be more aggressive
        // and drive the other one to a corner, so it's easier to checkmate, the mating king however should also
        // recognize this and try to stay in the center and not get pushed to the corner
        bool has_advantage = eval >= 500; // this is correct, if eval is positive, then this side has the advantage
        int who_has_advantage[2] = {-1, 1}, 
            lossing_king_sq[2] = {king_sq, eking_sq},
            winning_king_sq[2] = {eking_sq, king_sq};
        
        int advanatge_side = who_has_advantage[has_advantage];
        int losing_king = lossing_king_sq[has_advantage];
        int winning_king = winning_king_sq[has_advantage];
        int pos_eval = 0;
        if ((  pop_count(board->bitboards(is_white)[Board::PAWN_TYPE]) < 2 
             || pop_count(board->bitboards(is_enemy)[Board::PAWN_TYPE]) < 2) 
            && abs(eval) >= 500 && phase <= (total_phase_material >> 1)){
            // Either 1 or 0 pawns on one (or both) sides, and one side has a significant advantage
            // AND there are less or equal than half of the pieces on the board for example (Q + R vs R)
            // This is a good indicator that the game is heading towards the checkmate, and
            // king should help to push the enemy king to the corner

            // Favor losing king to be close to the corner
            pos_eval += center_manhattan_distance[losing_king] * 4 * advanatge_side;
            // Favor own king to be close to the enemy king
            pos_eval += manhattan_distance[winning_king][losing_king] * 2 * advanatge_side;
        }
        eval += pos_eval;

        // The result is a weighted average of the middle game and endgame evaluations
        return ((total_phase_material - phase) * endgame_eval + phase * eval) / total_phase_material;
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