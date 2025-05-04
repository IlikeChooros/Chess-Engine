#pragma once

#include "namespaces.hpp"
#include "ansi.hpp"
#include <cengine/board.h>

UI_NAMESPACE_BEGIN 


// Class to render the chess board in command line
class BoardRenderer : public Ansi
{
public:
    BoardRenderer() = default;
    ~BoardRenderer() = default;

    static void render(const chess::Board& board, bool side = 1, std::ostream& os = std::cout);

private:
    static void renderLine(const chess::Board& board, int row, bool side, std::ostream& os);
};

UI_NAMESPACE_END