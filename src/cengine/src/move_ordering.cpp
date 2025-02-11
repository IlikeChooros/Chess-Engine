#include <cengine/move_ordering.h>

namespace chess
{
    void MoveOrdering::sort(MoveList *ml, Move pv, Board *board, SearchCache* sc, Depth ply)
    {
        OrderedMove om[ml->size()];

        Eval::material_factors_t factors = Eval::get_factors(*board);
        Bitboard                  danger = board->generateDanger();

        // Set the values of each move
        for (size_t i = 0; i < ml->size(); i++)
        {
            om[i].set(
                ml->moves[i], pv, board, sc, ply, danger, 
                factors.endgame_factor, factors.middlegame_factor
            );
        }

        // Sort the moves, based on the value
        std::stable_sort(om, om + ml->size(), greater);

        // Update the ordering of the moves
        for (size_t i = 0; i < ml->size(); i++)
        {
            ml->moves[i] = om[i].move;
        }
    }
}