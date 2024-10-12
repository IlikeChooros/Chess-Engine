#pragma once

#include <algorithm>

#include "eval.h"
#include "move.h"
#include "board.h"
#include "transp_table.h"

namespace chess
{

    // Class to give value to moves, giving priority to PV moves, captures and history heuristic
    class OrderedMove
    {
        static inline int captured_value(Move m, Board* b)
        {
            if (m.isCapture())
            {
                // The value of the captured piece
                return 1024 + piece_values[Piece::getType(b->board[m.getTo()])] - piece_values[Piece::getType(b->board[m.getFrom()])];
            }
            return 0;
        }
    public:
        Move move = Move::nullMove;
        int value = 0;

        OrderedMove() = default;

        inline void set(const Move& m, const Move& pvm, Board* b, SearchCache* sc)
        {
            move = m;
            value = 0;
            value += (m == pvm) * 10000; // PV move, should be ordered first
            value += captured_value(m, b); // Most valuable victim, least valuable attacker
            value += sc->getHH().get(b->getSide() == Piece::White, m) * 25; // History heuristic
        }

        inline bool operator<(const OrderedMove& other) const
        {
            return value < other.value;
        }

        inline bool operator>(const OrderedMove& other) const
        {
            return value > other.value;
        }
    };

    void order_moves(MoveList *ml, MoveList *pv, Board *b, SearchCache* sc);
}