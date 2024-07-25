#pragma once


#include "types.h"

// Class representing a move, has a from, to and flags. Internally stored as a 32 bit integer
class Move
{
    public:

    // Mask for the from and to fields (6 bits)
    static const uint32_t MASK_MOVE = 0b111111;

    // Flag structure (4 bits)
    // first bit -> special bit 1 (0b000x)
    // second bit -> special bit 2 (0b00x0)
    // third bit -> capture (0b0x00)
    // fourth bit -> promotion (0bx000)
    static const uint32_t FLAG_NONE = 0b0000; // No flags
    static const uint32_t FLAG_DOUBLE_PAWN = 0b0001; // Double pawn move
    static const uint32_t FLAG_KING_CASTLE = 0b0010; // King castle 0b0010
    static const uint32_t FLAG_QUEEN_CASTLE = 0b0011; // Queen castle 0b0011

    // Capture flags
    static const uint32_t FLAG_CAPTURE = 0b0100;
    static const uint32_t FLAG_ENPASSANT_CAPTURE = 0b0101;

    // Promotion flags
    static const uint32_t FLAG_PROMOTION = 0b1000;
    static const uint32_t FLAG_KNIGHT_PROMOTION = 0b1000;
    static const uint32_t FLAG_BISHOP_PROMOTION = 0b1001;
    static const uint32_t FLAG_ROOK_PROMOTION = 0b1010;
    static const uint32_t FLAG_QUEEN_PROMOTION = 0b1011;

    // Promotion capture flags
    static const uint32_t FLAG_KNIGHT_PROMOTION_CAPTURE = 0b1100;
    static const uint32_t FLAG_BISHOP_PROMOTION_CAPTURE = 0b1101;
    static const uint32_t FLAG_ROOK_PROMOTION_CAPTURE = 0b1110;
    static const uint32_t FLAG_QUEEN_PROMOTION_CAPTURE = 0b1111;

    Move() : m_move(0) {};
    Move(uint32_t from, uint32_t to, uint32_t flags) {
        m_move = (flags << 12) | (from << 6) | to;
        // First 6 bits: to, next 6 bits: from, rest: flags (20 bits)
    };
    Move(uint32_t move) : m_move(move) {};
    Move(const Move& other) : m_move(other.m_move) {};

    uint32_t getFrom() const {return (m_move >> 6) & MASK_MOVE;};
    uint32_t getTo() const {return m_move & MASK_MOVE;};
    uint32_t getFlags() const {return m_move >> 12;};

    bool isCapture() const {return getFlags() & FLAG_CAPTURE;};
    bool isPromotion() const {return getFlags() & FLAG_PROMOTION;};
    bool isDoubleMove() const {return getFlags() == FLAG_DOUBLE_PAWN;};
    bool isEnPassant() const {return getFlags() == FLAG_ENPASSANT_CAPTURE;};
    bool isQueenCastle() const {return getFlags() == FLAG_QUEEN_CASTLE;};
    bool isKingCastle() const {return getFlags() == FLAG_KING_CASTLE;};
    bool isCastle() const {return isQueenCastle() || isKingCastle();};

    operator int() const {return m_move;};
    operator bool() const {return m_move != 0;};

    Move& operator=(const Move& other) {
        m_move = other.m_move;
        return *this;
    };

    bool operator==(const Move& other) const {
        return m_move == other.m_move;
    };

    bool operator!=(const Move& other) const {
        return m_move != other.m_move;
    };
    
    private:
    uint32_t m_move;
};