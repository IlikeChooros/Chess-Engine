#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    chess::Board board;
    board.loadFen("rnbq1k1r/pp2bppp/2pn2N1/8/2B5/8/PPP3PP/RNBQKR2 b Qkq - 7 12");
    ui::runWindow(board, argc, argv);
    return 0;
}