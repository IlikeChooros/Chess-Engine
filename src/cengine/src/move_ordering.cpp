#include <cengine/move_ordering.h>

namespace chess
{
    /**
     * @brief Order the moves in the move list, modifies the list in place
     */
    void order_moves(MoveList *ml, MoveList *pv, Board *board, SearchCache* sc)
    {
        Move pvm = pv && pv->size() > 0 ? Move(*pv->begin()) : Move();

        OrderedMove om[ml->size()];

        for(size_t i = 0; i < ml->size(); i++)
        {
            om[i].set((*ml)[i], pvm, board, sc);
        }

        std::sort(om, om + ml->size(), std::greater<OrderedMove>());

        for(size_t i = 0; i < ml->size(); i++)
        {
            ml->moves[i] = om[i].move;
        }
    }
}