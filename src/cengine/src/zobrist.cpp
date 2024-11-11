#include <cengine/zobrist.h>


namespace chess
{
    // Initialize the Zobrist hash keys

    int Zobrist::hash_pieces[2][6][64] = {0};
    int Zobrist::hash_castling[16] = {0};
    int Zobrist::hash_turn = 0;
    int Zobrist::hash_enpassant[8] = {0};
} // namespace chess