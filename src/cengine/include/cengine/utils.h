#pragma once

#include <iostream>
#include <bitset>
#include <string>
#include "settings.h"

/**
 * @brief Converts a square index to a string representation
 */
inline std::string square_to_str(int square, bool inverse = true){
    if(square < 0 || square > 63)
        return "??";
    std::string str(2, ' ');
    str[0] = 'a' + (square % 8);
    square = inverse ? 7 - square / 8 : square / 8;
    str[1] = '1' + square;
    return str;
}

/**
 * @brief Converts square as string to index format
 * @return -1 if the string is not a valid square
 */
inline int str_to_square(std::string str, bool inverse = true){
    if (str.size() < 2)
        return -1;
    if ((str[0] < 'a' || str[0] > 'h') || (str[1] < '1' || str[1] > '8'))
        return -1;
    int row = str[1] - '1';
    row = inverse ? 7 - row : row;
    return row * 8 + str[0] - 'a';
}

#if DEBUG_DETAILS

/**
 * @brief Prints a bitboard to the console
 */
inline void dbitboard(uint64_t bitboard64){
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

#else
inline void dbitboard(uint64_t bitboard64){return;}
#endif