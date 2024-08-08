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

    static uint64_t bishopAttacks[64][512];
    static uint64_t rookAttacks[64][4096];    
    
    static Magic bishopMagics[64];
    static Magic rookMagics[64];
};

void init_magics(bool recalculate = false);