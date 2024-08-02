#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    chess::Board board;
    init_board(&board);

    test::Perft p(&board);
    p.setPrint(true);

    p.setExpected(p.nodes_perft[5]);
    p.run(4);

    for(int i = 0; i < 6; i++)
    {
        auto test = test::PerftTestData::data[i];
        p.setExpected(test.nodes);
        p.run(test.depth, test.fen);
    }

    ui::runWindow(board, argc, argv);
    return 0;
}