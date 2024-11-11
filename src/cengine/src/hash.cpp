#include <cengine/hash.h>

namespace chess
{
    // Initialize the hashing tables
    void init_hashing()
    {
        constexpr int seed = 0x12345678;

        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>().max());
        
        for (int k = 0; k < 2; k++)
            for (int i = 0; i < 6; i++)
                for (int j = 0; j < 64; j++)
                    Zobrist::hash_pieces[k][i][j] = dist(gen);

        for (int i = 0; i < 16; i++)
            Zobrist::hash_castling[i] = dist(gen);
        
        Zobrist::hash_turn = dist(gen);

        for (int i = 0; i < 8; i++)
            Zobrist::hash_enpassant[i] = dist(gen);
    }

    // Generate a hash for a given board state
    uint64_t get_hash(Board* b)
    {
        uint64_t hash = 0;

        for(int type = 0; type < 6; type++){
            uint64_t white = b->bitboards(true)[type];
            uint64_t black = b->bitboards(false)[type];

            while(white){
                hash ^= Zobrist::hash_pieces[0][type][pop_lsb1(white)];
            }
            while(black){
                hash ^= Zobrist::hash_pieces[1][type][pop_lsb1(black)];
            }
        }

        if (b->getSide() == Piece::Black)
            hash ^= Zobrist::hash_turn;

        if (b->enpassantTarget() != 0)
            hash ^= Zobrist::hash_enpassant[b->enpassantTarget()];

        hash ^= Zobrist::hash_castling[b->castlingRights().get()];

        return hash;
    }

    // Generate a hash for a given pawn structure
    uint64_t get_pawn_hash(Board* board)
    {
        uint64_t hash = 0;

        uint64_t white = board->bitboards(true)[Piece::Pawn];
        uint64_t black = board->bitboards(false)[Piece::Pawn];

        while(white){
            hash ^= Zobrist::hash_pieces[0][Piece::Pawn - 1][pop_lsb1(white)];
        }
        while(black){
            hash ^= Zobrist::hash_pieces[1][Piece::Pawn - 1][pop_lsb1(black)];
        }

        return hash;
    }
}