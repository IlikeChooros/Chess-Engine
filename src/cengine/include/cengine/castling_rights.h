#pragma once


#include "types.h"

namespace chess
{
    class CastlingRights
    {
        int m_rights;

        public:
        static const int MASK = 0b1111;
        static const int NONE = 0;
        static const int WHITE_KING = 0b0001;
        static const int WHITE_QUEEN = 0b0010;
        static const int BLACK_KING = 0b0100;
        static const int BLACK_QUEEN = 0b1000;
        static const int BLACK = BLACK_KING | BLACK_QUEEN;
        static const int WHITE = WHITE_KING | WHITE_QUEEN;
        static const int ALL = 0b1111;


        CastlingRights() : m_rights(ALL) {};
        CastlingRights(int rights) : m_rights(rights) {};
        CastlingRights(const CastlingRights& other) : m_rights(other.m_rights) {};

        CastlingRights& operator=(const CastlingRights& other){
            m_rights = other.m_rights;
            return *this;
        };

        CastlingRights& operator=(int rights){
            m_rights = rights;
            return *this;
        };

        bool none() const {return (m_rights & MASK) == NONE;};
        bool has(int rights) const {return m_rights & rights;};
        void add(int rights) {m_rights |= rights;};
        void remove(int rights = ALL) {m_rights &= (~rights) & MASK;};

        operator int() const {return m_rights & MASK;};
        operator bool() const {return !none();};

        bool operator==(const CastlingRights& other) const {return m_rights == other.m_rights;};
        bool operator!=(const CastlingRights& other) const {return m_rights != other.m_rights;};

    };
}