#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    using namespace chess;
    Board board;
    board.init();
    // board.loadFen();

    ui::runWindow(board, argc, argv);
    return 0;
}