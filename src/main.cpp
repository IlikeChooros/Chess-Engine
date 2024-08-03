#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    uci::uciLoop(stdin, stdout);

    // using namespace chess;
    // Board board;
    // init_board(&board);
    // test::Perft p(&board);
    // p.setPrint(true);   
    // auto test = test::PerftTestData::data[9];
    // p.run(5, test.fen);
    // ui::runWindow(board, argc, argv);
    return 0;
}