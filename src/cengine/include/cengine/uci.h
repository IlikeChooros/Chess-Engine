#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "manager.h"
#include "test.h"

// Universal Chess Interface
namespace uci
{
    /**
     * @brief Start the UCI loop, will read from input and write to output
     * 
     * @param input The input stream
     * @param output The output stream
     */
    void uciLoop(FILE* input, FILE* output);
}

