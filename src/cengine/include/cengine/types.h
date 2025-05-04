#pragma once

#include "settings.h"
#include <cstdint>

namespace chess
{

typedef char Byte;
typedef uint64_t Bitboard;
typedef int Square;
typedef int16_t IScore;
typedef int Value;
typedef uint64_t Hash;
typedef int Depth;

typedef bool RepetitionType;
static constexpr RepetitionType Threefold = 0, Fivefold = 1;

enum NodeType
{
    Root,
    PV,
    nonPV
};

class squares
{
public:
    static constexpr Square getSquare(int file, int rank)
    {
        return ((7 - rank) << 3) + file;
    }
};

}

