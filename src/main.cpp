#include <cengine/cengine.h>
#include <ui/ui.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    chess::Manager::init();
    chess::Board board;
    board.init();
    // board.loadFen("r1bqr3/ppp1B1kp/1b4p1/n2B4/3PQ1P1/2P5/P4P2/RN4K1 w - - 1 0");

    ui::runWindow(board);
    return 0;
}