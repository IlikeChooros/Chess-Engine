#include <cengine/eval.h>


namespace chess
{
    // Source: https://www.chessprogramming.org/Simplified_Evaluation_Function
    // Added [0] -> middle game, [1] -> endgame tables, [6] -> type of a piece
    const int Eval::white_piece_square_table[2][6][64] = {
        {
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
            // king
            {-30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            20, 20,  0,  0,  0,  0, 20, 20,
            20, 30, 10,  0,  0, 10, 30, 20},
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
        },

        // endgame
        {
            //pawn
            {0,   0,  0,  0,  0,  0,  0,  0,
            55,  55, 50, 45, 45, 50, 55, 55,
            30,  30, 20, 30, 30, 20, 30, 30,
            15,  15, 20, 25, 25, 20, 20, 15,
            10,  10, 10, 10, 10, 10, 10, 10,
             5,   5,  5,  5,  5,  5,  5,  5,
            -10, -5, -5, -5, -5, -5, -5, -10,
            0,  0,  0,  0,  0,  0,  0,  0},

            // knight
            {-50,-40,-30,-30,-30,-30,-40,-50,
            -40,-20,  0,  0,  0,  0,-20,-40,
            -30,  0, 10, 15, 15, 10,  0,-30,
            -30,  5, 15, 20, 20, 15,  5,-30,
            -30,  0, 15, 20, 20, 15,  0,-30,
            -30,  5, 10, 15, 15, 10,  5,-30,
            -40,-20,  0,  5,  5,  0,-20,-40,
            -50,-40,-30,-30,-30,-30,-40,-50},

            // king
            {-50,-40,-30,-20,-20,-30,-40,-50,
            -30,-20,-10,  0,  0,-10,-20,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-10, 30, 40, 40, 30,-10,-30,
            -30,-10, 30, 40, 40, 30,-10,-30,
            -30,-10, 20, 30, 30, 20,-10,-30,
            -30,-30,  0,  0,  0,  0,-30,-30,
            -50,-30,-30,-30,-30,-30,-30,-50},

            // bishop
            {-20,-10,-10,-10,-10,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -20,-10,-10,-10,-10,-10,-10,-20},

            // rook
            {10, 10, 10, 10, 10, 10, 10, 10,
             10, 20, 20, 20, 20, 20, 20, 10,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  0,  0,  0,  0,  0,-10,
              0,  0,  0, 10, 10,  0,  0,  0},

            // queen
            {-20,-10,-10, -5, -5,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5,  5,  5,  5,  0,-10,
             -5,  0,  5,  5,  5,  5,  0, -5,
              0,  0,  5,  5,  5,  5,  0, -5,
            -10,  5,  5,  5,  5,  5,  0,-10,
            -10,  0,  5,  0,  0,  0,  0,-10,
            -20,-10,-10, -5, -5,-10,-10,-20},
        }
    };

    // Modifiable piece values
    int Eval::piece_square_table[2][2][6][64] = {0};
    Bitboard Eval::passed_pawn_masks[2][64] = {0};
    int8_t Eval::manhattan_distance[64][64] = {0};
    TTable<int> Eval::pawn_table = TTable<int>(4);

    /**
     * @brief Initialize the boards for evaluation
     */
    void Eval::init()
    {
        // Initialize the pieces tables
        for (int state = 0; state < 2; state++){
            for (int type = 0; type < 6; type++){
                for (int j = 0; j < 64; j++){
                    piece_square_table[state][1][type][j] = white_piece_square_table[state][type][j];
                    piece_square_table[state][0][type][j] = white_piece_square_table[state][type][63 - j];
                }
            }
        }

        // Initialize the passed pawn masks
        for(int side = 0; side < 2; side++)
        {
            for(Square sq = 0; sq < 64; sq++)
            {
                int rank = sq >> 3;
                int file = sq % 8;
                Bitboard file_bb = 0;

                // Iterate through this square and it's sides
                for (int i = -1; i < 2; i++)
                {
                    // File out of board bounds
                    if (file + i < 0 || file + i > 7)
                        continue;

                    // Get file bitboard for this square
                    file_bb |= file_bitboards[file + i];

                    // Delete all bits BELOW or equal to this rank
                    if (side == 0)
                        for (int r = 0; r <= rank && rank < 7; r++)
                            file_bb &= ~(1UL << (8*r + file + i));
                    // Delete all bits ABOVE or eqaul to this rank
                    else 
                        for (int r = rank; r < 8 && rank > 0; r++)
                            file_bb &= ~(1UL << (8*r + file + i));
                }

                passed_pawn_masks[side][sq] = file_bb;
            }
        }

        // Initalize manhattan distance
        for (int i = 0; i < 64; i++)
        {
            int rank = i / 8;
            int file = i % 8;

            for (int j = 0; j < 64; j++)
            {
                int rank2 = j / 8;
                int file2 = j % 8;

                manhattan_distance[i][j] = abs(rank - rank2) + abs(file - file2);
            }
        }
    }

    /**
     * @brief Evaluate pawn structure
     */
    constexpr int eval_pawn_structure(Bitboard pawns, Bitboard epawns, Bitboard file, bool is_white)
    {
        int pawn_eval = 0;
        Bitboard pawns_on_file = pawns & file;

        if (pawns_on_file)
        {
            // Doubled pawns
            if (pop_count(pawns_on_file) > 1)
                pawn_eval -= 10;

            // Isolated pawns
            if (!(pawns & (file >> 1)) && !(pawns & (file << 1)))
                pawn_eval -= 10;
            
            // Connected
            if (pawns & (file >> 8) || pawns & (file << 8))
                pawn_eval += 10;

            // Get square of that pawn and check if it's a passed pawn
            while (pawns_on_file)
                if ((Eval::passed_pawn_masks[is_white][pop_lsb1(pawns_on_file)] & epawns) == 0)
                    pawn_eval += 30;
        }

        return pawn_eval;
    }

    
    /**
     * @brief Evaluation function for the board in centipawns
     * positive values are good current side, negative for the opposite
     */
    int Eval::evaluate(Board& board)
    {
        int eval           = 0;
        int material       = 0;
        bool is_white      = board.getSide() == Piece::White;
        bool is_enemy      = !is_white;
        int endgame_factor = 0;

        // Calcualte endgame factor
        for(int type = 1; type < 6; type++)
        {
            if (type == Piece::King - 1)
                continue;
            
            // Calculate endgame factor, by adding up values of a piece * it's quantity
            int my_count    = pop_count(board.m_bitboards[is_white][type]);
            int enemy_count = pop_count(board.m_bitboards[is_enemy][type]);

            endgame_factor += (ENDGAME_FACTOR_PIECES[type] * (my_count + enemy_count));

            // Update the evaluation
            material += piece_values[type] * (my_count - enemy_count);
        }

        eval += material;

        // Clamp the value from 0 to MAX_ENDGAME_FACTOR
        endgame_factor = std::min(endgame_factor, MAX_ENDGAME_FACTOR);
        endgame_factor = MAX_ENDGAME_FACTOR - endgame_factor;

        // Count the material & piece square tables
        int square_table_eval = 0;
        for (int type = 0; type < 6; type++){
            uint64_t epieces = board.bitboards(is_enemy)[type];
            uint64_t apieces = board.bitboards(is_white)[type];

            while(epieces){
                int sq = pop_lsb1(epieces);
                square_table_eval -= (endgame_factor * piece_square_table[is_enemy][ENDGAME][type][sq]
                        + (MAX_ENDGAME_FACTOR - endgame_factor) * piece_square_table[is_enemy][MIDDLE_GAME][type][sq]); 
            }
            while(apieces){
                int sq = pop_lsb1(apieces);
                square_table_eval += (endgame_factor * piece_square_table[is_white][ENDGAME][type][sq]
                        + (MAX_ENDGAME_FACTOR - endgame_factor) * piece_square_table[is_white][MIDDLE_GAME][type][sq]);
            }
        }

        // Scale the square table evaluation
        square_table_eval /= MAX_ENDGAME_FACTOR;

        // Add the evaluation to the total
        eval += square_table_eval;

        // Bonus for having the bishop pair
        if (pop_count(board.bitboards(is_white)[Piece::Bishop - 1]) >= 2){
            eval += 50;
        }
        if (pop_count(board.bitboards(is_enemy)[Piece::Bishop - 1]) >= 2){
            eval -= 50;
        }

        // Pawn structure
        // Try to get hashed pawn structure (already calculated)
        Bitboard pawn_hash = board.pawnHash();
        if (pawn_table.contains(pawn_hash)){
            eval += pawn_table.get(pawn_hash);
        } else {
            int pawn_eval = 0;

            Bitboard pawns  = board.m_bitboards[is_white][Piece::Pawn - 1];
            Bitboard epawns = board.m_bitboards[is_enemy][Piece::Pawn - 1];

            for (int i = 0; i < 8; i++)
            {
                Bitboard file = file_bitboards[i];
                pawn_eval += eval_pawn_structure(pawns, epawns, file, is_white);
                pawn_eval -= eval_pawn_structure(epawns, pawns, file, is_enemy);
            }

            // Store the pawn hash
            pawn_table.store(pawn_hash, pawn_eval);
            eval += pawn_eval;
        }

        // if the endgame factor is high and one side has a significant advantage
        // then the winning side's king should be more active and 
        // try to get closer to the enemy king
        if (
            endgame_factor > (MAX_ENDGAME_FACTOR * 8 / 10) && abs(material) >= piece_values[Piece::Rook - 1]
            && ((board.m_bitboards[0][Piece::Pawn - 1] | board.m_bitboards[1][Piece::Pawn - 1]) == 0))
        {
            int whoiswinning  = material > 0 ? 1 : -1;
            Square king       = bitScanForward(board.m_bitboards[is_white][Piece::King - 1]);
            Square enemy_king = bitScanForward(board.m_bitboards[is_enemy][Piece::King - 1]);

            // If it's winning side, apply penalty for being far from the enemy king
            // for losing side, apply bonus for being far from the enemy king
            eval += -13 * whoiswinning * (manhattan_distance[king][enemy_king]);
        }

        return eval;
    }
}