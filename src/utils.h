#pragma once

#include <iostream>
#include <bitset>
#include <string>
#include "settings.h"

/**
 * @brief Converts a square index to a string representation
 */
inline std::string square_to_str(int square){
    std::string str(2, ' ');
    str[0] = 'a' + (square % 8);
    str[1] = '1' + (7 - square / 8);
    return str;
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