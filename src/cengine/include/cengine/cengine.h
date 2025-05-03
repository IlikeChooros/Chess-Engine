#pragma once

/*
    cengine - a simple chess engine in C++

    The MIT License (MIT)
    SPDX short identifier: MIT

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files 
    (the "Software"), to deal in the Software without restriction, 
    including without limitation the rights to use, copy, modify, merge, publish, 
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom 
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "settings.h"
#include "perft.h"
#include "move.h"
#include "uci.h"
#include "magic_bitboards.h"
#include "pgn.h"

namespace chess
{
    /**
     * Initialize the boards, search and evaluation
     * 
     * ### CEngine usage
     * 
     * 
     * ```cpp
     * 
     * #include <cengine/cengine.h>
     * 
     * int main()
     * {
     *    // Initialize the engine
     *    chess::init();
     *   
     * 
     *   // Create a new `Engine` instance
     *   chess::Engine engine;
     *   engine.position(chess::START_FEN);
     *   
     *   // Search options
     *   SearchOptions options;
     *   options["depth"] = 5;
     *   options["movetime"] = 3000;
     * 
     *   // Start the search
     *   chess::Move best_move = engine.go(options);
     *   engine.join();
     *   
     *   std::cout << "Best move: " << best_move.uci() << "\n";
     * 
     *  return 0;
     * }
     * ```
     * 
     */
    inline void init()
    {    
        Engine::base_init();
    }
}