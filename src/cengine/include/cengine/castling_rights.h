#pragma once


#include "types.h"

namespace chess
{
    class CastlingRights
    {
        uint32_t m_rights;

        public:
        static const uint32_t MASK = 0b1111;
        static const uint32_t NONE = 0;
        static const uint32_t WHITE_KING = 0b0001;
        static const uint32_t WHITE_QUEEN = 0b0010;
        static const uint32_t BLACK_KING = 0b0100;
        static const uint32_t BLACK_QUEEN = 0b1000;
        static const uint32_t BLACK = BLACK_KING | BLACK_QUEEN;
        static const uint32_t WHITE = WHITE_KING | WHITE_QUEEN;
        static const uint32_t ALL = 0b1111;
        static const int bits = 4;


        CastlingRights() : m_rights(ALL) {};
        CastlingRights(uint32_t rights) : m_rights(rights) {};
        CastlingRights(const CastlingRights& other) : m_rights(other.m_rights) {};

        CastlingRights& operator=(const CastlingRights& other){
            m_rights = other.m_rights;
            return *this;
        };

        CastlingRights& operator=(uint32_t rights){
            m_rights = rights;
            return *this;
        };

        bool none() const {return (m_rights & MASK) == NONE;};
        bool has(int rights) const {return m_rights & rights;};
        void add(int rights) {m_rights |= rights;};
        void remove(int rights = ALL) {m_rights &= (~rights) & MASK;};
        uint32_t rights() const {return m_rights;};

        operator int() const {return m_rights & MASK;};
        operator bool() const {return !none();};

        bool operator==(const CastlingRights& other) const {return m_rights == other.m_rights;};
        bool operator!=(const CastlingRights& other) const {return m_rights != other.m_rights;};

    };
}