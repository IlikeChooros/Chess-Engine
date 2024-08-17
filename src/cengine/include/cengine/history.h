#pragma once

#include <list>

#include "board.h"
#include "pieces.h"
#include "move.h"
#include "castling_rights.h"
#include "types.h"
#include "hash.h"

namespace chess
{
    struct CHistory
    {
        uint64_t hash; // 64 bits for Zobrist hash
        uint32_t move:Move::bits; // 16 bits
        uint32_t side_to_move:Piece::bits; // 5 bit for side to move (Piece::Color)
        uint32_t captured_piece:Piece::bits; // 5 bits for piece
        uint32_t enpassant_target:6; // 6 bits for square (0 - 63)
        uint32_t halfmove_clock:6; // 6 bits for halfmove clock (0 - 63 (max is 50))
        uint32_t fullmove_counter:13; // 13 bits for fullmove counter (0 - 8192)
        uint32_t castling_rights:CastlingRights::bits; // 3 bits for castling rights
        uint32_t reserved:9;
        // That gives total of 128 bits, instead of 6*32 + 64 = 256 bits
    };

    class GameHistory
    {
    public:

        GameHistory(Board* b = nullptr)
        {
            history.reserve(256);
            if (b != nullptr)
                push(b, Move());
        }

        GameHistory(const GameHistory& other) 
        {
            history = other.history;
        }

        GameHistory& operator=(const GameHistory& other) 
        {
            history = other.history;
            return *this;
        }

        void clear() 
        {
            history.clear();
        }

        size_t ply() 
        {
            return history.size() - 1;
        }

        int age() 
        {
            return history.size() - 1;
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
            h.hash = get_hash(board);
            h.move = move.move();
            h.side_to_move = board->getSide();
            h.captured_piece = board->capturedPiece();
            h.enpassant_target = board->enpassantTarget();
            h.halfmove_clock = board->halfmoveClock();
            h.fullmove_counter = board->fullmoveCounter();
            h.castling_rights = board->castlingRights().get();
            history.push_back(h);
        }

        int repetitions(Board* b, uint64_t hash)
        {
            int count = 0;
            for (size_t i = b->irreversibleIndex(); i < history.size(); i++){
                if (history[i].hash == hash)
                    count++;
            }
            return count;
        }

        std::vector<CHistory> history;
    };
}

