#pragma once

#include <memory>
#include <string.h>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "pieces.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"


namespace chess{
    class Board
    {
    public:
        static const int mailbox64[64];
        static const int mailbox[120];
        static const int piece_move_offsets[6][8];
        static const int pawn_attack_offsets[2][2];
        static const int pawn_move_offsets[2][2];
        static const int n_piece_rays[6];
        static const bool is_piece_sliding[6];

        Board() = default;
        Board(const Board& other) = delete;
        
        Board& init();
        void loadFen(const char* fen);
        std::string getFen();
        void updateBitboards();

        /**
         * @brief Get the side to move
         */
        inline int& getSide() {return this->m_side; };

        /**
         * @brief Get the enpassant target square
         */
        inline int& enpassantTarget() {return this->m_enpassant_target; };
        
        /**
         * @brief Get the halfmove clock
         */
        inline int& halfmoveClock() {return this->m_halfmove_clock; };

        /**
         * @brief Get the fullmove counter
         */
        inline int& fullmoveCounter() {return this->m_fullmove_counter; };

        /**
         * @brief Get the castling rights
         */
        inline CastlingRights& castlingRights() {return this->m_castling_rights; };

        /**
         * @brief Get raw board data
         */
        inline int* getBoard() {return this->board.get(); };

        /**
         * @brief Get the bitboards for a given color
         */
        inline uint64_t* bitboards(bool is_white) {return this->m_bitboards[is_white]; };

        /**
         * @brief Moves given piece from one square to another on the bitboard
         */
        inline void updateBitboard(int is_white, int type, int from, int to){
            this->m_bitboards[is_white][type] ^= (1ULL << from) | (1ULL << to);
        }

        /**
         * @brief Get the piece at a given index
         */
        inline int& operator[](int index) {return this->board[index]; };

        std::vector<int> findAll(int piece);
        

        std::unique_ptr<int[]> board;
        int m_side;
        int m_enpassant_target;
        int m_halfmove_clock;
        int m_fullmove_counter;
        uint64_t m_bitboards[2][6]; // 0: white, 1: black, contains bitboards for each piece type
        CastlingRights m_castling_rights;
    };
}
