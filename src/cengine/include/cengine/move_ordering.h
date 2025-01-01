#pragma once

#include <algorithm>

#include "eval.h"
#include "move.h"
#include "board.h"
#include "transp_table.h"

namespace chess
{

    // Move ordering for move lists, based on the 
    // pricipal variation, captures and history heuristic
    class MoveOrdering
    {
    public:
        MoveOrdering() = default;

        // Class to give value to moves, giving priority to PV moves, captures and history heuristic
        class OrderedMove
        {
            static inline int captured_value(Move m, Board* b)
            {
                if (m.isCapture())
                {
                    // The value of the captured piece
                    return 900 
                        + Eval::piece_values[Piece::getType(b->board[m.getTo()]) - 1] 
                        - Eval::piece_values[Piece::getType(b->board[m.getFrom()]) - 1];
                }
                return 0;
            }

        public:
            Move move = Move::nullMove;
            int value = 0;

            OrderedMove() = default;

            // Set the move and its value
            inline void set(const Move& m, const Move& pvm, Board* b, SearchCache* sc)
            {
                move = m;
                value = 0;
                value += (m == pvm) * 16000; // PV move, should be ordered first
                value += captured_value(m, b); // Most valuable victim, least valuable attacker
                value += sc->getHH().get(b->getSide() == Piece::White, m) * 256; // History heuristic
            }
        };

        static constexpr bool greater(const OrderedMove& __x, const OrderedMove& __y)
        {
            return __x.value > __y.value;
        }

        /**
         * @brief Order the moves in the move list, modifies the list in place
         */
        static void sort(MoveList *ml, Move pv, Board *b, SearchCache* sc);
    };
}