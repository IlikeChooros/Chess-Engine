#include "src/ui.h"

int main(int argc, char** argv)
{
    chess::Board board;
    board.init();
    ui::runWindow(board, argc, argv);
    return 0;
}