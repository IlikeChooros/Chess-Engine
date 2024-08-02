#pragma once

#include <list>

#include "board.h"
#include "pieces.h"
#include "move.h"
#include "castling_rights.h"
#include "types.h"

namespace chess
{
    struct CHistory
    {
        uint32_t move:Move::bits; // 16 bits
        uint32_t side_to_move:Piece::bits; // 5 bit for side to move (Piece::Color)
        uint32_t captured_piece:Piece::bits; // 5 bits for piece
        uint32_t enpassant_target:6; // 7 bits for square (0 - 63)
        uint32_t halfmove_clock:6; // 6 bits for halfmove clock (0 - 63 (max is 50))
        uint32_t fullmove_counter:10; // 11 bits for fullmove counter (0 - 1023)
        uint32_t castling_rights:CastlingRights::bits; // 3 bits for castling rights
        uint32_t game_state:2; // 2 bits for game state
        uint32_t reserved:10;
        // That gives total of 64 bits, instead of 6*32 = 192 bits
    };

    struct GameHistory
    {
        std::list<CHistory> history;

        void clear() 
        {
            history.clear();
        }

        size_t size() 
        {
            return history.size();
        }

        CHistory& back()
        {
            return history.back();
        }

        void pop() 
        {
            history.pop_back();
        }

        void push(Board* board, Move move) 
        {
            CHistory h;
            h.move = move.move();
            h.side_to_move = board->getSide();
            h.captured_piece = board->capturedPiece();
            h.enpassant_target = board->enpassantTarget();
            h.halfmove_clock = board->halfmoveClock();
            h.fullmove_counter = board->fullmoveCounter();
            h.castling_rights = board->castlingRights().get();
            h.game_state = 0;
            history.push_back(h);
        }
    };
}

