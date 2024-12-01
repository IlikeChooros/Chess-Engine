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

void init_magics(bool recalculate = false);

} // namespace chess