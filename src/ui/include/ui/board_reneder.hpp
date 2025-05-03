#pragma once

#include "namespaces.hpp"
#include <cengine/board.h>

UI_NAMESPACE_BEGIN 


// Class to render the chess board in command line
class BoardRenderer
{
public:
    BoardRenderer() = default;
    ~BoardRenderer() = default;

    static void render(const chess::Board& board, std::ostream& os = std::cout);    
};

UI_NAMESPACE_END