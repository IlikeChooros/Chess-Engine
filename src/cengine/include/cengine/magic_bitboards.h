#pragma once

/*
Heavily referenced from:
https://www.chessprogramming.org/Looking_for_Magics
*/

#include <random>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <fstream>

#include "threads.h"
#include "types.h"
#include "utils.h"
#include "mailbox.h" // for move mailbox move generation
#include "pieces.h"

namespace chess
{
// Magic bitboard struct, fancy magic bitboard stuff
struct Magic {
    Bitboard mask;
    Bitboard magic;
    int shift;
};

class MagicBitboards
{
public:
    static const int RBits[64];
    static const int BBits[64];

    static Bitboard bishopAttacks[64][512];
    static Bitboard rookAttacks[64][4096];    
    
    static Magic bishopMagics[64];
    static Magic rookMagics[64];
};

/**
 * @brief Get the bishop attacks, based on the magic bitboards
 */
inline uint64_t bishopAttacks(uint64_t occupied, int sq)
{
    auto& magic = MagicBitboards::bishopMagics[sq];
    return MagicBitboards::bishopAttacks[sq][((occupied & magic.mask) * magic.magic) >> magic.shift];
}

/**
 * @brief Get the rook attacks, calculated using magic bitboards
 */
inline uint64_t rookAttacks(uint64_t occupied, int sq)
{
    auto& magic = MagicBitboards::rookMagics[sq];
    return MagicBitboards::rookAttacks[sq][((occupied & magic.mask) * magic.magic) >> magic.shift];
}

/**
 * @brief Get the queen attacks, using magic bitboards
 */
inline uint64_t queenAttacks(uint64_t occupied, int sq)
{
    return rookAttacks(occupied, sq) | bishopAttacks(occupied, sq);
}

void init_magics(bool recalculate = false);

} // namespace chess