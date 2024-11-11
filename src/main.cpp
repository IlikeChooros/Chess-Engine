#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    // uci::UCI uci;
    // uci.loop();

    using namespace chess;

    init();

    Board board;
    board.loadFen("6k1/5pp1/1Q2b2p/4P3/7P/8/3r2PK/3q4 w - - 1 34 moves b6b4 d1e2 b4f4 e2e5");
    board.print();
    return 0;
}