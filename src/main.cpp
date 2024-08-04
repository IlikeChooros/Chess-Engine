#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    // uci::uciLoop(stdin, stdout);

    using namespace chess;
    Board board;
    board.init();

    ui::runWindow(board, argc, argv);
    return 0;
}