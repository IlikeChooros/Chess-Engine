#include <cengine/hash.h>

namespace chess
{

    uint64_t Zobrist::hash_values[64 * 12 + 16 + 8 + 1];

    // Initialize the hashing tables
    void Zobrist::init_hashing()
    {
        constexpr int seed = 0x21376942;

        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>().max());
        
        // Initialize piece hashes
        for (int k = 0; k < 2; k++) // color
            for (int i = 0; i < 6; i++) // piece type
                for (int j = 0; j < 64; j++) // square
                    hash_values[k * 6 * 64 + i * 64 + j] = dist(gen);

        // Castling rights
        for (int i = 0; i < 16; i++) 
            hash_values[64 * 12 + i] = dist(gen);

        // Enpassant target
        for (int i = 0; i < 8; i++)
            hash_values[64 * 12 + 16 + i] = dist(gen);

        // Turn
        hash_values[64 * 12 + 16 + 8] = dist(gen);
    }

    // Generate a hash for a given board state
    uint64_t Zobrist::get_hash(Board* b)
    {
        uint64_t hash = 0;
        for (int k = 0; k < 2; k++){
            for (int i = 0; i < 6; i++){
                uint64_t pieces = b->bitboards(k)[i];
                while(pieces){
                    hash ^= hash_values[k * 6 * 64 + i * 64 + pop_lsb1(pieces)];
                }
            }
        }
        hash ^= hash_values[64 * 12 + b->castlingRights().get()];

        int target = b->enpassantTarget();
        if (target != 0)
            hash ^= hash_values[64 * 12 + 16 + target % 8]; // get the file

        hash ^= hash_values[64 * 12 + 16 + 8];

        return hash;
    }

    // Generate a hash for a given pawn structure
    uint64_t Zobrist::get_pawn_hash(Board* board)
    {
        uint64_t hash = 0;
        for(int k = 0; k < 2; k++){
            uint64_t pawns = board->bitboards(k)[Board::PAWN_TYPE];
            while(pawns){
                hash ^= hash_values[k * 6 * 64 + Board::PAWN_TYPE * 64 + pop_lsb1(pawns)];
            }
        }
        return hash;
    }
}