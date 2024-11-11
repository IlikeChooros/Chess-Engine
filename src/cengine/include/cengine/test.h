#pragma once

#include <algorithm>

#include "manager.h"
#include "pgn.h"
#include "perft.h"

namespace test
{
    using namespace chess;

    class PerftTestData
    {
    public:
        typedef struct 
        {
            uint32_t depth;
            uint64_t nodes;
            std::string fen;
        } PerftData;

        static const PerftData data[27];
    };
}