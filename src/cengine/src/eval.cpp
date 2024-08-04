#include <cengine/eval.h>


namespace chess
{
    // Source: https://www.chessprogramming.org/Simplified_Evaluation_Function
    const int piece_values[6] = {100, 320, 330, 500, 900, 20000};
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
    static int ***piece_square_table = nullptr;
    static int ***king_square_tables = nullptr;

    /**
     * @brief Initialize the boards for evaluation
     */
    void init_eval()
    {
        piece_square_table = new int**[2];
        piece_square_table[0] = new int*[6];
        piece_square_table[1] = new int*[6];

        king_square_tables = new int**[2];
        king_square_tables[0] = new int*[2];
        king_square_tables[1] = new int*[2];

        // Initialize the black piece square tables
        for(int i = 0; i < 6; i++){
            piece_square_table[0][i] = new int[64];
            piece_square_table[1][i] = new int[64];
            for(int j = 0; j < 64; j++){
                piece_square_table[1][i][j] = white_piece_square_table[i][j];
                piece_square_table[0][i][j] = white_piece_square_table[i][63 - j];
            }
        }

        // Initialize the black king square tables
        for(int i = 0; i < 2; i++){
            king_square_tables[0][i] = new int[64];
            king_square_tables[1][i] = new int[64];
            for(int j = 0; j < 64; j++){
                king_square_tables[1][i][j] = white_king_square_tables[i][j];
                king_square_tables[0][i][j] = white_king_square_tables[i][63 - j];
            }
        }
    }

    
    /**
     * @brief Evaluation function for the board in centipawns
     */
    int evaluate(Board* board)
    {
        int eval = 0;
        bool is_white = board->getSide() == Piece::White;
        bool is_enemy = !is_white;
        int whotomove[2] = {-1, 1};

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

        // if (eval == 45){
        //     printf("%s", board->getFen().c_str());
        //     dbitboard(board->occupied(false));
        //     dbitboard(board->occupied(true));
        // }

        return eval * whotomove[is_white];
    }
}