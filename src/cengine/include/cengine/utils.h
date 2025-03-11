#pragma once

#include <iostream>
#include <bitset>
#include <string>
#include <chrono>
#include <vector>
#include <queue>
#include <emmintrin.h>

#include "types.h"
#include "settings.h"

namespace chess
{

/**
 * @brief Converts a square index to a string representation
 */
inline std::string square_to_str(Square square, bool inverse = true)
{
    if(square < 0 || square > 63)
        return "??";

    std::string str(2, ' ');
    str[0] = 'a' + (square % 8);
    square = inverse ? 7 - square / 8 : square / 8;
    str[1] = '1' + (char)square;
    return str;
}

/**
 * @brief Converts square as string to index format
 * @return -1 if the string is not a valid square
 */
inline Square str_to_square(std::string str, bool inverse = true)
{
    if (str.size() < 2)
        return -1;

    if ((str[0] < 'a' || str[0] > 'h') || (str[1] < '1' || str[1] > '8'))
        return -1;

    int row = str[1] - '1';
    row = inverse ? 7 - row : row;
    return row * 8 + str[0] - 'a';
}

/**
 * @brief Calculate the dot product (sum of b_i * weight_i, where b_i - is the ith bit (0 <= i < 64))
 * @param bb the bitboard (most likely the moves of a piece)
 * @param weights 64 byte array, [0] -> is the top left corner square, [64]-> bottom right corner
 */
constexpr int dot_product(Bitboard bb, Byte weights[])
{
    int product = 0;
    for(int sq = 0, bit = 1; sq < 64; sq++, bit += bit)
    {
        // Could write:
        // if (bb & bit == 0) product += weights[sq]
        // but this is branchless, uses the fact -1 == 111111111...1 (all bits set to 1)
        // and -0 = 0 = 0000000000...0 (all bits set to 0), 
        // so weights[sq] & (-1) = weights[sq], and weights[sq] & (-0) = 0.
        product += weights[sq] & (-((bb & bit) == 0));
    }
    return product;
}

/**
 * @brief Get the LSB1 index (the least significant bit that is 1)
 */
inline int bit_scan_forward(Bitboard bb)
{
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_ctzll(bb);
    #elif defined(_MSC_VER)
        unsigned long index;
        _BitScanForward64(&index, bb);
        return index;
    #else
        // BitScanForward implementation for other compilers
        // Using de Bruijn multiplication
        // https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
        const uint64_t magic = 0x022fdd63cc95386d; // the 4061955.

        const unsigned int magictable[64] =
        {
            0,  1,  2, 53,  3,  7, 54, 27,
            4, 38, 41,  8, 34, 55, 48, 28,
            62,  5, 39, 46, 44, 42, 22,  9,
            24, 35, 59, 56, 49, 18, 29, 11,
            63, 52,  6, 26, 37, 40, 33, 47,
            61, 45, 43, 21, 23, 58, 17, 10,
            51, 25, 36, 32, 60, 20, 57, 16,
            50, 31, 19, 15, 30, 14, 13, 12,
        };

        return magictable[((bb&-bb)*magic) >> 58];
    #endif
}

/**
 * @brief Pop the least significant bit that is 1 from a bitboard and return its index
 */
inline int pop_lsb1(uint64_t& b)
{
    int lsb1 = b != 0 ? bit_scan_forward(b) : 0;
    b &= b - 1;
    return lsb1;
}

inline int unsafe_pop_lsb1(Bitboard& b)
{
    int lsb = bit_scan_forward(b);
    b &= b - 1;
    return lsb;
}

/**
 * @brief Count the number of bits that are 1 in a bitboard
 */
inline int pop_count(uint64_t b)
{
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_popcountll(b);
    #elif defined(_MSC_VER)
        return __popcnt64(b);
    #else
        // Popcount implementation for other compilers
        // Using Brian Kernighan's Algorithm
        int count = 0;
        while (b) {
            b &= b - 1;
            count++;
        }
        return count;
    #endif
}

/**
 * @brief Prints a bitboard to the standard output
 */
inline void dbitboard(uint64_t bitboard64)
{
    std::bitset<64> bitboard(bitboard64);
    for(int i = 0; i < 64; i++){
        printf("%*d", 2, int(bitboard[i]));
        if(i % 8 == 7)
            printf(" |%c \n", '8' - i / 8);
    }
    for(int i = 0; i < 8; i++)
        printf("%*c", 2, 'a' + i);
    printf("\n\n");
}


} // namespace chess