#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    using namespace chess;
    Board board;
    init_board(&board);
    // board.loadFen("8/8/8/4k3/3P2K1/2p5/B7/8 b - d3 0 3");

    test::Perft p(&board);
    p.setPrint(true);   

    for(int i = 0; i < 23; i++)
    {
        auto test = test::PerftTestData::data[i];
        p.setExpected(test.nodes);
        p.run(test.depth, test.fen);
    }

    ui::runWindow(board, argc, argv);
    return 0;
}