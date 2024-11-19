#pragma once

#include "settings.h"
#include <cstdint>

typedef uint64_t Bitboard;
typedef int Square;
typedef int16_t IScore;
typedef int Value;
typedef uint64_t Hash;

typedef bool RepetitionType;
static constexpr RepetitionType Threefold = 0, Fivefold = 1;

typedef bool NodeType;
static constexpr NodeType Root = 1, nonRoot = 0;