#include <cengine/move_ordering.h>

namespace chess
{
    inline int captured_value(Move m, Board* b)
    {
        if (m.isCapture())
        {
            // The value of the captured piece
            return piece_values[Piece::getType(b->board[m.getTo()])] - piece_values[Piece::getType(b->board[m.getFrom()])];
        }
        return 0;
    }

    // Static exchange evaluation, returns the value of the move
    int see(int square, Board* b)
    {
        return 0;
    }
    
    inline bool sort_captures(Move a, Move b, Board* board)
    {
        // Most valuable victim, least valuable attacker
        return captured_value(a, board) > captured_value(b, board);
    }

    /**
     * @brief Sort moves by captures, then by history heuristic, returns true if the move a should be ordered before b
     */
    bool sort_moves(Move a, Move b, Move pv, Board* board, bool side, SearchCache* sc)
    {
        // If a move is the PV move, it should be ordered first
        bool apv = a == pv;
        if (apv || b == pv)
            return a == pv;
        // If a move is a capture, it should be ordered first
        bool acp = a.isCapture(), bcp = b.isCapture();
        if (acp && bcp)
            return sort_captures(a, b, board);
        if (acp || bcp)
            return acp;
        // Sort non-captures by history heuristic
        return sc->getHH().get(side, a) > sc->getHH().get(side, b);
    }

    void order_moves(MoveList *ml, MoveList *pv, Board *board, SearchCache* sc)
    {
        bool side = board->wside();
        Move pvm = pv && pv->size() > 0 ? Move(*pv->begin()) : Move();
        std::sort(ml->begin(), ml->end(), [&board, &sc, &pvm, side](const Move &a, const Move &b) {
            return sort_moves(a, b, pvm, board, side, sc);
        });
    }
}