#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    // uci::UCI backend;
    // backend.loop();

    // For profiling
    TimeManagement tm;

    chess::Board b;
    test::Perft p(&b);
    p.run(6);
    std::cout << "Perft: " << tm.elapsed_us() << "us" << std::endl;

    return 0;
}