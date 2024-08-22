#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    uci::UCI backend;
    backend.loop();

    // For profiling perft
    // TimeManagement tm; 
    // chess::Board b;
    // test::Perft p(&b);
    // p.run(6);
    // std::cout << "Perft: " << tm.elapsed_us() << "us" << std::endl;

    // For profiling search
    // chess::Board b;
    // b.loadFen("r2q1rk1/pp3ppp/2n2n2/6B1/3p4/P2Q4/1PP1NPPP/R4RK1 b - - 2 14");
    // chess::Manager m(&b);
    // chess::Manager::init();

    // chess::SearchParams sp;
    // sp.infinite = false;
    // sp.movetime = 3000;
    // m.impl()->setSearchParams(sp);
    // m.search();
    return 0;
}