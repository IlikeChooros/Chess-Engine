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
            20, 30, 20,  0,  0, 10, 30, 20},
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
        }
    };

    // Modifiable piece values
    // evaluate piece based on it's position, [turn][state][type][square]
    int Eval::piece_square_table[2][2][6][64] = {0};

    // 
    Bitboard Eval::passed_pawn_masks[2][64] = {0};

    // manhattan distance [from|to][to|from] (symetrical)
    int8_t Eval::manhattan_distance[64][64] = {0};

    // Hash table for pawn structure
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
                    piece_square_table[1][state][type][j] = white_piece_square_table[state][type][j];
                    piece_square_table[0][state][type][j] = white_piece_square_table[state][type][63 - j];
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
    int eval_pawn_structure(Bitboard pawns, Bitboard epawns, Bitboard file, bool is_white)
    {
        int pawn_eval          = 0;
        Bitboard pawns_on_file = pawns & file;
        int rank_offset        = is_white ? -1 : 1;

        if (pawns_on_file)
        {
            // Doubled pawns
            if (pop_count(pawns_on_file) > 1)
                pawn_eval -= 20;

            // Isolated pawns
            if (!(pawns & (file >> 1)) && !(pawns & (file << 1)))
                pawn_eval -= 15;

            // Get square of that pawn and check if it's a passed pawn
            do
            {
                Square sq           = pop_lsb1(pawns_on_file);
                Square rank         = sq >> 3;
                Bitboard rank_below = Eval::rank_bitboards[rank + rank_offset];

                // Pawn structures (positive)
                // pawn chain
                if (pawns & (((file >> 1) | (file << 1)) & rank_below))
                    pawn_eval += 5;
                
                // side-to-side
                if (pawns & (((file >> 1) | (file << 1)) & Eval::rank_bitboards[rank]))
                    pawn_eval += 5;

                // passer
                if ((Eval::passed_pawn_masks[is_white][sq] & epawns) == 0)
                    pawn_eval += 15;
                    // pawn_eval += Eval::piece_square_table[is_white][Eval::ENDGAME][Piece::Pawn - 1][sq];

            } while(pawns_on_file);
        }

        return pawn_eval;
    }

    /**
     * @brief Calculate king safety based on pawn structure near king
     */
    int eval_king_safety(Bitboard pawns, Square king_sq, bool is_white)
    {
        int      eval        = 0;
        int      offset_file = 0;
        Square   file        = king_sq % 8;
        Square   rank        = king_sq >> 3;
        Bitboard rank_off    = is_white ? 
            (rank > 0 ? Eval::rank_bitboards[rank - 1] : 0) 
            : (rank < 8 ? Eval::rank_bitboards[rank + 1] : 0);

        // Cut off the area below the king (based on the side)
        Bitboard safety      = (Board::pieceAttacks[Board::KING_TYPE][king_sq] & (~rank_off)) & pawns;

        for (int i = -1; i < 2; ++i)
        {
            offset_file = file + i;
            if ((offset_file >= 0 && offset_file < 8) 
                && (safety & Eval::file_bitboards[offset_file]) != 0)
            {
                eval += 5;
            }
        }
        
        return eval;
    }

    // Old evaluation function
    int old_eval(Board& board)
    {
        int eval        = 0;
        bool is_white   = board.getSide() == Piece::White;
        bool is_enemy   = !is_white;
        Square king_sq  = bitScanForward(board.bitboards(is_white)[Piece::King - 1]);
        // Square eking_sq = bitScanForward(board.bitboards(is_enemy)[Piece::King - 1]);

        // Count the material & piece square tables
        for (int type = 0; type < 6; type++){
            if (type == Piece::King - 1){
                continue;
            }

            uint64_t epieces = board.bitboards(is_enemy)[type];
            uint64_t apieces = board.bitboards(is_white)[type];

            while(epieces){
                int sq = pop_lsb1(epieces);
                eval -= Eval::piece_values[type];
                eval -= Eval::piece_square_table[is_enemy][Eval::MIDDLE_GAME][type][sq];
            }
            while(apieces){
                int sq = pop_lsb1(apieces);
                eval += Eval::piece_values[type];
                eval += Eval::piece_square_table[is_white][Eval::MIDDLE_GAME][type][sq];
            }
        }
        
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
        if (Eval::pawn_table.contains(pawn_hash))
        {
            eval += Eval::pawn_table.get(pawn_hash);
        } 
        else 
        {
            int pawn_eval = 0;
            Bitboard pawns = board.bitboards(is_white)[Piece::Pawn - 1];
            Bitboard epawns = board.bitboards(is_enemy)[Piece::Pawn - 1];

            for (int i = 0; i < 8; i++){
                Bitboard file = Eval::file_bitboards[i];

                // Doubled pawns
                if (pawns & file && pop_count(pawns & file) > 1){
                    pawn_eval -= 10;
                }
                if (epawns & file && pop_count(epawns & file) > 1){
                    pawn_eval += 10;
                }

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
            }

            // Store the pawn hash
            Eval::pawn_table.store(pawn_hash, pawn_eval);
            eval += pawn_eval;
        }        

        // King square tables
        bool is_endgame = false;

        if (pop_count(board.pieces()) <= 6 || ((pop_count(board.queens()) == 0) && (pop_count(board.rooks()) == 0))){
            is_endgame = true;
        }

        eval += Eval::piece_square_table[is_white][is_endgame][Piece::King - 1][king_sq];
        eval -= Eval::piece_square_table[is_enemy][is_endgame][Piece::King - 1][king_sq];

        return eval;
    }

    
    /**
     * @brief Evaluation function for the board in centipawns
     * positive values are good current side, negative for the opposite
     */
    int Eval::evaluate(Board& board)
    {
        int eval              = 0;
        int material          = 0;
        int endgame_factor    = 0;
        int middlegame_factor = 0;
        bool is_white         = board.getSide() == Piece::White;
        bool is_enemy         = !is_white;
        // Square king_sq        = bitScanForward(board.bitboards(is_white)[Piece::King - 1]);

        // Step 1: Evaluate the material and middlegame/endgame factors
        for(int type = Piece::Pawn - 1; type < 6; type++)
        {
            if (type == Piece::King - 1)
                continue;
            
            // Calculate endgame factor, by adding up values of a piece * it's quantity
            int my_count    = pop_count(board.m_bitboards[is_white][type]);
            int enemy_count = pop_count(board.m_bitboards[is_enemy][type]);

            middlegame_factor += (ENDGAME_FACTOR_PIECES[type] * (my_count + enemy_count));

            // Update the material
            material += piece_values[type] * (my_count - enemy_count);
        }

        eval += material;

        // Clamp the value from 0 to MAX_ENDGAME_FACTOR
        middlegame_factor = std::min(middlegame_factor, MAX_ENDGAME_FACTOR);
        endgame_factor    = MAX_ENDGAME_FACTOR - middlegame_factor;

        // Step 2: Evaluate the piece square tables
        int square_table_eval = 0;
        for (int type = 0; type < 6; type++)
        {
            uint64_t epieces = board.m_bitboards[is_enemy][type];
            uint64_t apieces = board.m_bitboards[is_white][type];

            while(epieces)
            {
                Square sq = pop_lsb1(epieces);
                square_table_eval -=
                            (endgame_factor * piece_square_table[is_enemy][ENDGAME][type][sq]
                        + middlegame_factor * piece_square_table[is_enemy][MIDDLE_GAME][type][sq]); 
            }
            while(apieces)
            {
                Square sq = pop_lsb1(apieces);
                square_table_eval +=
                            (endgame_factor * piece_square_table[is_white][ENDGAME][type][sq]
                        + middlegame_factor * piece_square_table[is_white][MIDDLE_GAME][type][sq]);
            }
        }
        
        square_table_eval /= MAX_ENDGAME_FACTOR;
        eval += square_table_eval;
        
        // Bonus for having the bishop pair
        if (pop_count(board.bitboards(is_white)[Piece::Bishop - 1]) >= 2){
            eval += 50;
        }
        if (pop_count(board.bitboards(is_enemy)[Piece::Bishop - 1]) >= 2){
            eval -= 50;
        }

        // Step 3: Evaluate the pawn structure
        // Try to get hashed pawn structure (already calculated)
        Bitboard pawn_hash = board.pawnHash();
        if (Eval::pawn_table.contains(pawn_hash))
        {
            eval += Eval::pawn_table.get(pawn_hash);
        } 
        else 
        {
            int pawn_eval = 0;
            Bitboard pawns = board.bitboards(is_white)[Piece::Pawn - 1];
            Bitboard epawns = board.bitboards(is_enemy)[Piece::Pawn - 1];

            for (int i = 0; i < 8; i++){
                Bitboard file = Eval::file_bitboards[i];

                pawn_eval += eval_pawn_structure(pawns, epawns, file, is_white);
                pawn_eval -= eval_pawn_structure(epawns, pawns, file, is_enemy);

                // Doubled pawns
                // if (pawns & file && pop_count(pawns & file) > 1){
                //     pawn_eval -= 10;
                // }
                // if (epawns & file && pop_count(epawns & file) > 1){
                //     pawn_eval += 10;
                // }

                // // Isolated pawns
                // if (pawns & file){
                //     if (!(pawns & (file >> 1)) && !(pawns & (file << 1))){
                //         pawn_eval -= 10;
                //     }
                // }
                // if (epawns & file){
                //     if (!(epawns & (file >> 1)) && !(epawns & (file << 1))){
                //         pawn_eval += 10;
                //     }
                // }
            

                // // Connected pawns
                // if (pawns & file){
                //     if (pawns & (file >> 8) || pawns & (file << 8)){
                //         pawn_eval += 10;
                //     }
                // }
                // if (epawns & file){
                //     if (epawns & (file >> 8) || epawns & (file << 8)){
                //         pawn_eval -= 10;
                //     }
                // }
            }

            // Store the pawn hash
            Eval::pawn_table.store(pawn_hash, pawn_eval);
            eval += pawn_eval;
        }

        // if (
        //     material >= piece_values[Piece::Rook - 1]
        //     && ((board.m_bitboards[0][Piece::Pawn - 1] | board.m_bitboards[1][Piece::Pawn - 1]) == 0))
        // {
        //     Square king       = bitScanForward(board.m_bitboards[is_white][Piece::King - 1]);
        //     Square enemy_king = bitScanForward(board.m_bitboards[is_enemy][Piece::King - 1]);

        //     // If it's winning side, apply penalty for being far from the enemy king
        //     // for losing side, apply bonus for being far from the enemy king
        //     eval += -10 * (manhattan_distance[king][enemy_king]);
        // }

        return eval;


        // int eval              = 0;
        // int material          = 0;
        // bool is_white         = board.getSide() == Piece::White;
        // bool is_enemy         = !is_white;
        // int endgame_factor    = 0;
        // int middlegame_factor = 0;


        // // Step 1: Evaluate the material and middlegame/endgame factors
        // for(int type = Piece::Pawn - 1; type < 6; type++)
        // {
        //     if (type == Piece::King - 1)
        //         continue;
            
        //     // Calculate endgame factor, by adding up values of a piece * it's quantity
        //     int my_count    = pop_count(board.m_bitboards[is_white][type]);
        //     int enemy_count = pop_count(board.m_bitboards[is_enemy][type]);

        //     middlegame_factor += (ENDGAME_FACTOR_PIECES[type] * (my_count + enemy_count));

        //     // Update the evaluation
        //     material += piece_values[type] * (my_count - enemy_count);
        // }

        // eval += material;

        // // Clamp the value from 0 to MAX_ENDGAME_FACTOR
        // middlegame_factor = std::min(middlegame_factor, MAX_ENDGAME_FACTOR);
        // endgame_factor    = MAX_ENDGAME_FACTOR - middlegame_factor;

        
        // // Step 2: Evaluate the piece square tables

        // int square_table_eval = 0;
        // for (int type = 0; type < 6; type++)
        // {
        //     uint64_t epieces = board.m_bitboards[is_enemy][type];
        //     uint64_t apieces = board.m_bitboards[is_white][type];

        //     while(epieces)
        //     {
        //         Square sq = pop_lsb1(epieces);
        //         square_table_eval -=
        //                     (endgame_factor * piece_square_table[is_enemy][ENDGAME][type][sq]
        //                 + middlegame_factor * piece_square_table[is_enemy][MIDDLE_GAME][type][sq]); 
        //     }
        //     while(apieces)
        //     {
        //         Square sq = pop_lsb1(apieces);
        //         square_table_eval +=
        //                     (endgame_factor * piece_square_table[is_white][ENDGAME][type][sq]
        //                 + middlegame_factor * piece_square_table[is_white][MIDDLE_GAME][type][sq]);
        //     }
        // }

        // // Scale the square table evaluation
        // square_table_eval /= MAX_ENDGAME_FACTOR;

        // // Add the evaluation to the total
        // eval += square_table_eval;

        // // Step 2a: Bonus for having both bishops

        // if (pop_count(board.bitboards(is_white)[Piece::Bishop - 1]) >= 2){
        //     eval += 50;
        // }
        // if (pop_count(board.bitboards(is_enemy)[Piece::Bishop - 1]) >= 2){
        //     eval -= 50;
        // }

        // // Step 3: Evaluate the pawn structure
        // // Try to get hashed pawn structure (already calculated)
        // Bitboard pawn_hash = board.pawnHash();
        // if (pawn_table.contains(pawn_hash)){
        //     eval += pawn_table.get(pawn_hash);
        // } else {
        //     int pawn_eval = 0;

        //     Bitboard pawns  = board.m_bitboards[is_white][Piece::Pawn - 1];
        //     Bitboard epawns = board.m_bitboards[is_enemy][Piece::Pawn - 1];

        //     for (int i = 0; i < 8; i++)
        //     {
        //         Bitboard file = file_bitboards[i];
        //         pawn_eval += eval_pawn_structure(pawns, epawns, file, is_white);
        //         pawn_eval -= eval_pawn_structure(epawns, pawns, file, is_enemy);
        //     }

        //     // Store the pawn hash
        //     pawn_table.store(pawn_hash, pawn_eval);
        //     eval += pawn_eval;
        // }

        // Square king       = bitScanForward(board.m_bitboards[is_white][Piece::King - 1]);
        // Square enemy_king = bitScanForward(board.m_bitboards[is_enemy][Piece::King - 1]);

        // // Step 4: Evaluate the king safety
        // eval += eval_king_safety(board.m_bitboards[is_white][Board::PAWN_TYPE], king, is_white);
        // eval -= eval_king_safety(board.m_bitboards[is_enemy][Board::PAWN_TYPE], enemy_king, is_enemy);


        // // Step 5: Evaluate the king position in the winning endgame
        // // if the endgame factor is high and one side has a significant advantage
        // // then the winning side's king should be more active and 
        // // try to get closer to the enemy king
        // if (
        //     material >= piece_values[Piece::Rook - 1]
        //     && ((board.m_bitboards[0][Piece::Pawn - 1] | board.m_bitboards[1][Piece::Pawn - 1]) == 0))
        // {
        //     // If it's winning side, apply penalty for being far from the enemy king
        //     // for losing side, apply bonus for being far from the enemy king
        //     eval += -10 * (manhattan_distance[king][enemy_king]);
        // }

        // return eval;
    }
}