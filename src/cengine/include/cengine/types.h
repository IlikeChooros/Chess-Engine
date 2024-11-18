#pragma once

#include "settings.h"
#include <cstdint>

typedef uint64_t Bitboard;
typedef int Square;
typedef int16_t IScore;
typedef int Value;
typedef uint64_t Hash;

typedef int NodeType;
static constexpr int Root = 1, nonRoot = 0;