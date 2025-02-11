#pragma once

#include "cache.h"
#include "board.h"
#include "types.h"
#include "eval.h"

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
    static bool valid(Depth ply, Move& move, bool check_before_move, bool impr, SearchCache* sc)
    {
        return !(impr || sc->getKH().is_killer(move, ply)
            || (!move.isQuiet()) || check_before_move);
    }

    // Returns new reduce value of the search depth
    constexpr static int reduce(Depth depth, int n_move, bool null_window)
    {
        return ((null_window ? 2 : 1) + depth / 3 + n_move / 13) + 1;
    }
};

// Null Move Pruning (Heuristic)
class NMP
{
public:

    // Checks if given position is valid for pruning
    static bool valid(Depth depth, Board& board)
    {
        if ((depth < 4))
            return false;

        auto factors = Eval::get_factors(board);
        return ((factors.endgame_factor < (int)(Eval::MAX_ENDGAME_FACTOR * 0.9))
                && !board.m_in_check);
    }

    static int reduce(Depth depth)
    {
        return 3 + depth / 4;
    }
};

}