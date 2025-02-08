#pragma once

#include "cache.h"
#include "board.h"
#include "types.h"

namespace chess
{

// Late move reductions
class LMR
{
public:

    // Returns new depth of the search
    // Reduces based on these criteria:
    // - depth >= 3
    // - board is not in check
    // - move is not a killer (caused beta cut-off)
    static int reduce(Depth depth, Depth ply, Move& move, Board& state, SearchCache* sc)
    {
        // Do not reduce on small depth, when in check, or if the move is a killer
        bool not_reducing = (depth < 3) || state.m_in_check || sc->getKH().is_killer(move, ply);

        if (not_reducing)
            return depth;
        
        // Decrease by 1
        return depth - 1;
    }
};

}