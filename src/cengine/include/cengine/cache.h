#pragma once

#include "types.h"

// Cached move generation information
namespace cache
{
    struct CacheMoveGen
    {
        // Attacks by enemy pieces
        uint64_t danger;
    };
}