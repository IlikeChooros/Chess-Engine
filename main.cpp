#include "src/ui.h"

int main()
{
    chess::Board board;
    board.init();
    ui::runWindow(board);
    return 0;
}