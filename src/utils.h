#pragma once

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
#   define dbitboard(__bitboard64) do { \
    for (int i = 0; i < 8; i++) { \
        for (int j = 0; j < 8; j++) { \
            printf("%*lu", 2, (__bitboard64 >> (i * 8 + j)) & 1); \
        } \
        printf("\n"); \
    } \
} while(0)
#else
#   define dbitboard(__bitboard64) do {} while(0)
#endif