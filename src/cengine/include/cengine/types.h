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

}

