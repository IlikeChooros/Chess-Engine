#pragma once

#include "history.h"
#include "move.h"
#include "board.h"
#include "utils.h"

void init_board(chess::Board* board);
void verify_castling_rights(chess::Board* board);

uint64_t mailboxAttacks(int type, uint64_t occupied, int square, bool is_sliding = true);
uint64_t mailboxPawnMoves(uint64_t occupied, int square, bool is_white);

uint64_t rookAttacks(uint64_t occupied, int square);
uint64_t bishopAttacks(uint64_t occupied, int square);
uint64_t xRayRookAttacks(uint64_t occupied, uint64_t blockers, int square);
uint64_t xRayBishopAttacks(uint64_t occupied, uint64_t blockers, int square);

size_t gen_legal_moves(MoveList* move_list, chess::Board* board);

void make(Move& move, chess::Board* board, chess::GameHistory* ghistory);
void unmake(Move& move, chess::Board* board, chess::GameHistory* history);
