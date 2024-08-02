#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <chrono>

#include "board.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"
#include "move_gen.h"
#include "history.h"

namespace chess
{
    class ManagerImpl
    {
    public:

        static uint64_t attacks_to[2][64];
        static uint64_t attacks_from[2][64];
        static const int castling_data[2][4];
        static const int castling_offsets[2][2];
        static const int castling_flags[2];

        typedef enum{
            Normal = 0,
            Checkmate = 1,
            Draw = 2,
            Repetition = 3,
        } GameState;

        ManagerImpl(Board* board = nullptr);
        ManagerImpl(const ManagerImpl& other) = delete;
        ManagerImpl& operator=(ManagerImpl&& other);

        void init();
        void reload();
        int generateMoves();
        void make(Move& move);
        void unmake();
        GameState evalState();
        // int generatePseudoMoves(bool is_white, uint64_t occupied, uint64_t enemy_pieces, int* move_list);
        // bool validateMove(Move& move, int king_pos, bool is_white, uint64_t pinnedbb, uint64_t pinners, uint64_t in_between_bb);
        // void addMove(int from, int to, int flags, int* move_list, int& n_moves);
        // void addAttack(int from, int to, bool piece_is_white);
        // void handleCapture(Move& move);
        // void handleMove(Move& move);
        // void checkKingCastling(bool is_white, int j, int king_index);
        // void handleCastlingMove(bool is_king_castle, int from, int to);
        void pushHistory();

        /**
         * @brief Validate castling rights, should be called once after loading a FEN string
         */
        inline void validateCastlingRights() { verify_castling_rights(board); }

        GameHistory history;
        std::vector<uint32_t> move_list;
        int n_moves;
        Board* board;
        Move curr_move;
        int captured_piece;
        GameState state;  
    };
}