#include <cengine/hash.h>

namespace chess
{

    static int hash_pieces[2][6][64] = {0};
    static int hash_castling[16] = {0};
    static int hash_turn = 0;
    static int hash_enpassant[8] = {0};


    // Initialize the hashing tables
    void init_hashing()
    {
        const int seed = 0x12345678;

        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>().max());
        
        for (int k = 0; k < 2; k++)
            for (int i = 0; i < 6; i++)
                for (int j = 0; j < 64; j++)
                    hash_pieces[k][i][j] = dist(gen);

        for (int i = 0; i < 16; i++)
            hash_castling[i] = dist(gen);
        
        hash_turn = dist(gen);

        for (int i = 0; i < 8; i++)
            hash_enpassant[i] = dist(gen);
    }

    // Generate a hash for a given board state
    uint64_t get_hash(Board* b)
    {
        uint64_t hash = 0;

        for(int type = 0; type < 6; type++){
            uint64_t white = b->bitboards(true)[type];
            uint64_t black = b->bitboards(false)[type];

            while(white){
                hash ^= hash_pieces[0][type][pop_lsb1(white)];
            }
            while(black){
                hash ^= hash_pieces[1][type][pop_lsb1(black)];
            }
        }

        if (b->getSide() == Piece::Black)
            hash ^= hash_turn;

        if (b->enpassantTarget() != 0)
            hash ^= hash_enpassant[b->enpassantTarget()];

        hash ^= hash_castling[b->castlingRights().get()];

        return hash;
    }
}