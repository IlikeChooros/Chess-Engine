#pragma once

#include <iostream>
#include <functional>
#include <memory>
#include <string.h>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "state.h"
#include "pieces.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"
#include "zobrist.h"
#include "mailbox.h"
#include "magic_bitboards.h"

namespace chess
{

    enum Termination
    {
        NONE = 0,
        CHECKMATE,
        STALEMATE,
        DRAW,
        INSUFFICIENT_MATERIAL,
        FIFTY_MOVES,
        THREEFOLD_REPETITION,
        FIVEFOLD_REPETITION,
        SEVENTYFIVE_MOVES,
        SEVENTYFIVE_MOVES_NO_CAPTURE,
        SEVENTYFIVE_MOVES_NO_PAWN_MOVE,
        SEVENTYFIVE_MOVES_NO_CAPTURE_NO_PAWN_MOVE,
        TIME,
        RESIGNATION,
        UNKNOWN
    };

    /**
     * ## Board
     * 
     * The board class represents the chess board and its state.
     * Uses both mailbox and bitboard representation.
     * 
     */
    class Board: public Mailbox
    {
        friend class Engine;

        inline void restore_state(State& state);
        inline void undo(Square from, Square to, bool is_white, int type);
        void push_state(Move move);
        void verify_castling_rights();
        static void init_board();

    public:
        typedef MoveList::move_filter_t MoveFilter;

        // Helper bitboards initialized by `init_board()`
        static Bitboard in_between[64][64];
        static Bitboard pawnAttacks[2][64];
        static Bitboard pieceAttacks[6][64];

        // Starting position
        static const char START_FEN[57];

        // Piece types
        static constexpr int PAWN_TYPE   = Piece::Pawn - 1;
        static constexpr int ROOK_TYPE   = Piece::Rook - 1;
        static constexpr int BISHOP_TYPE = Piece::Bishop - 1;
        static constexpr int QUEEN_TYPE  = Piece::Queen - 1;
        static constexpr int KING_TYPE   = Piece::King - 1;
        static constexpr int KNIGHT_TYPE = Piece::Knight - 1;

        Board();
        Board(const Board& other);
        Board(std::string fen);
        Board& operator=(const Board& other);
        
        Board& init();
        void loadFen(std::string fen);
        std::string fen();
        void updateBitboards();
        void makeNullMove();
        void undoNullMove();
        void makeMove(Move move);
        void undoMove(Move move);
        Hash hash();
        Hash pawnHash();

        /**
         * @brief Get the termination of the game, it doesn't calculate it
         */
        Termination getTermination() const {return this->m_termination; };

        /**
         * @brief Get the hash of the board, doesn't calculate it
         */
        Hash getHash() const {return this->m_hash; };

        bool isTerminated();
        bool isTerminated(MoveList* ml);
        template <RepetitionType type = Threefold>
        bool isRepetition();
        bool isInsufficientMaterial();
        bool isDraw();
        bool isCheckmate(MoveList* ml);
        bool isStalemate(MoveList* ml);


        bool isLegal(Move move);
        Move match(Move move);

        Bitboard generateDanger();
        inline MoveList generateEvasions(Bitboard danger);
        MoveList generateLegalCaptures();
        MoveList generateLegalMoves();
        MoveList filterMoves(MoveFilter filter);

        void print();
        bool checkIntegrity();

        /**
         * @brief Get the side to move
         * @return bool True if white, false if black
         */
        inline bool turn() const {return this->m_side == Piece::White; };

        /**
         * @brief Get the side to move (Piece::Color)
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
         * @brief Get the irreversible index
         */
        inline int& irreversibleIndex() {return this->m_irreversible_index; };

        /**
         * @brief Get the fullmove counter
         */
        inline int& fullmoveCounter() {return this->m_fullmove_counter; };

        /**
         * @brief Get the castling rights
         */
        inline CastlingRights& castlingRights() {return this->m_castling_rights; };

        /**
         * @brief Get the captured piece
         */
        inline int& capturedPiece() {return this->m_captured_piece; };

        /**
         * @brief Get bool flag wheter the king is in check
         */
        inline bool& inCheck() {return this->m_in_check; };

        /**
         * @brief Get raw board data
         */
        inline int* getBoard() {return this->board; };

        /**
         * @brief Get the history of the game
         */
        inline StateList& history() {return this->m_history; };

        /**
         * @brief Get the bitboards for a given color
         */
        inline Bitboard* bitboards(bool is_white) {return this->m_bitboards[is_white]; };

        /**
         * @brief Get the occupied squares for a given color
         */
        inline Bitboard occupied(bool is_white) 
        {
            Bitboard color = 0;
            for (int i = 0; i < 6; i++){
                color |= this->m_bitboards[is_white][i];
            }
            return color;
        }

        /**
         * @brief Get the occupied squares on the board
         */
        inline Bitboard occupied() 
        {
            Bitboard occ = 0;
            for (int i = 0; i < 6; i++){
                occ |= this->m_bitboards[0][i] | this->m_bitboards[1][i];
            }
            return occ;
        }

        /**
         * @brief Get the opposite rooks / queen bitboard
         */
        inline Bitboard oppRooksQueens(bool is_white) {
            return this->m_bitboards[!is_white][ROOK_TYPE] | this->m_bitboards[!is_white][QUEEN_TYPE];
        }

        /**
         * @brief Get the opposite bishops / queen bitboard
         */
        inline Bitboard oppBishopsQueens(bool is_white) {
            return this->m_bitboards[!is_white][BISHOP_TYPE] | this->m_bitboards[!is_white][QUEEN_TYPE];
        }

        /**
         * @brief Get the pieces bitboard
         */
        inline Bitboard pieces() 
        {
            return m_bitboards[0][KNIGHT_TYPE] | m_bitboards[0][ROOK_TYPE] | m_bitboards[0][BISHOP_TYPE] | m_bitboards[0][QUEEN_TYPE] |
                   m_bitboards[1][KNIGHT_TYPE] | m_bitboards[1][ROOK_TYPE] | m_bitboards[1][BISHOP_TYPE] | m_bitboards[1][QUEEN_TYPE];
        }

        inline Bitboard pawns() {
            return m_bitboards[0][PAWN_TYPE] | m_bitboards[1][PAWN_TYPE];
        }

        /**
         * @brief Get the queens bitboard
         */
        inline Bitboard queens() {
            return m_bitboards[0][QUEEN_TYPE] | m_bitboards[1][QUEEN_TYPE];
        }

        /**
         * @brief Get the rooks bitboard
         */
        inline Bitboard rooks() {
            return m_bitboards[0][ROOK_TYPE] | m_bitboards[1][ROOK_TYPE];
        }
        
        /**
         * @brief Moves given piece from one square to another on the bitboard
         */
        inline void updateBitboard(int is_white, int type, int from, int to){
            this->m_bitboards[is_white][type] ^= (1ULL << from | 1ULL << to);
        }

        /**
         * @brief Get the piece at a given index
         */
        inline int& operator[](int index) {return this->board[index]; };
        
        Hash m_hash;
        int board[64];
        bool m_in_check;
        int m_side;
        int m_enpassant_target;
        int m_halfmove_clock;
        int m_fullmove_counter;
        int m_captured_piece;
        int m_irreversible_index; // last irreversible move index
        Bitboard m_bitboards[2][6]; // 0: white, 1: black, contains bitboards for each piece type
        CastlingRights m_castling_rights;
        StateList m_history;
        Termination m_termination;
    };
}
