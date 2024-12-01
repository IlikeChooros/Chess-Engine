#pragma once

#include "board.h"
#include "types.h"


namespace chess
{


// Extensions class, used to extend the search in certain conditions
class Extensions
{
public:
    static constexpr int MAX_EXTENSIONS = 4;

    /**
     * @brief Extend the search by 1 ply if the position is in check,
     * up to a maximum of MAX_EXTENSIONS
     * @param board The board to check
     * @param n_check_extensions The number of check extensions to apply
     */
    static int check(Board& board, int& extension)
    {
        if (extension < MAX_EXTENSIONS && board.inCheck())
        {
            extension++;
            return 1;
        }

        return 0;
    }

};


}