#include <cengine/zobrist.h>


namespace chess
{
    // Initialize the Zobrist hash keys
    Hash Zobrist::hash_pieces[2][6][64] = {0};
    Hash Zobrist::hash_castling[16] = {0};
    Hash Zobrist::hash_turn = 0;
    Hash Zobrist::hash_enpassant[8] = {0};


    /**
     * @brief Initialize the Zobrist hash keys
     */
    void init_hashing()
    {
        constexpr int seed = 0x12345678;

        std::mt19937 gen(seed);
        std::uniform_int_distribution<Hash> dist(0, std::numeric_limits<Hash>().max());
        
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
} // namespace chess