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
    
    inline bool sort_captures(Move a, Move b, Board* board, CacheMoveGen* cache)
    {
        // Most valuable victim, least valuable attacker
        return captured_value(a, board) < captured_value(b, board);
    }

    /**
     * @brief Sort moves by captures, then by history heuristic, returns true if the move a should be ordered before b
     */
    bool sort_moves(Move a, Move b, Board* board, CacheMoveGen* cache, SearchCache* sc)
    {
        // If a move is a capture, it should be ordered first
        if (a.isCapture() && b.isCapture())
            return sort_captures(a, b, board, cache);
        if (a.isCapture() || b.isCapture())
            return b.isCapture();
        // Sort non-captures by history heuristic
        bool side = board->getSide() == Piece::White;
        return sc->getHH().get(side, a) < sc->getHH().get(side, b);
    }

    void order_moves(MoveList *ml, Board *board, CacheMoveGen* cache, SearchCache* sc)
    {
        std::sort(ml->begin(), ml->end(), [&board, &cache, &sc](const Move &a, const Move &b) {
            return sort_moves(a, b, board, cache, sc);
        });
    }
}