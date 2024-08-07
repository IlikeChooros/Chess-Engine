#pragma once

#include <random>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>

#include "threads.h"
#include "types.h"
#include "utils.h"
#include "board.h" // for move mailbox move generation

// Magic bitboard struct, fancy magic bitboard stuff
struct Magic {
    uint64_t mask;
    uint64_t magic;
    int shift;
};

class MagicBitboards
{
public:
    static const int RBits[64];
    static const int BBits[64];

    static uint64_t bishopAttacks[512];
    static uint64_t rookAttacks[4096];    
    
    static Magic bishopMagics[64];
    static Magic rookMagics[64];
};

void init_magics(bool recalculate = false);