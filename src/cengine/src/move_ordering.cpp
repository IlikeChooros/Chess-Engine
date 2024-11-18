#include <cengine/move_ordering.h>

namespace chess
{
    void MoveOrdering::sort(MoveList *ml, MoveList *pv, Board *board, SearchCache* sc)
    {
        Move pvm = pv && pv->size() > 0 ? Move(*pv->begin()) : Move();
        
        OrderedMove om[ml->capacity()];

        for(size_t i = 0; i < ml->size(); i++)
        {
            om[i].set((*ml)[i], pvm, board, sc);
        }

        std::stable_sort(om, om + ml->size(), std::greater<OrderedMove>());

        for(size_t i = 0; i < ml->size(); i++)
        {
            ml->moves[i] = om[i].move;
        }
    }
}