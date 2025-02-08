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

            /**
             * @brief Set the move and its value
             * @param m Move to set the Value
             * @param pvm hash move for this position (null-move if non-existent)
             * @param b board state
             * @param sc search cache has history and killer heuristic stored  
             * @param ply Current distance from the `root` position (depth of the search)
             */
            inline void set(const Move& m, const Move& pvm, Board* b, SearchCache* sc, Depth ply = 0)
            {
                move = m;
                value = 0;
                value += int(m == pvm) * 32000; // PV move, should be ordered first
                value += int(sc->getKH().is_killer(m, ply)) * 20000; // this is a killer move, should be ordered 2nd
                value += captured_value(m, b); // Most valuable victim, least valuable attacker
                value += sc->getHH().get(b->getSide() == Piece::White, m) * 128; // History heuristic
            }
        };

        static constexpr bool greater(const OrderedMove& __x, const OrderedMove& __y)
        {
            return __x.value > __y.value;
        }

        /**
         * @brief Order the moves in the move list, modifies the list in place
         */
        static void sort(MoveList *ml, Move pv, Board *b, SearchCache* sc, Depth ply = 0);
    };
}