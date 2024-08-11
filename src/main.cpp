#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    using namespace chess;
    uci::UCI u;
    u.loop();

    // test::TestGamePlay t;
    // t.run();

    Board board;
    board.init();
    // board.loadFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

    ui::runWindow(board, argc, argv);
    return 0;
}