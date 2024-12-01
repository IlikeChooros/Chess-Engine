#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    uci::UCI uci;
    uci.loop(argc, argv);

    // chess::init();
    // chess::Engine engine;
    // engine.setPosition("8/8/8/r7/8/7K/2k5/8 b - - 0 1 moves a5a4 h3g3 a4a5 g3h3 a5a4 h3g3 a4a5 g3h3");
    // std::cout << chess::Eval::evaluate(engine.m_board) * (engine.m_board.turn() ? 1 : -1) << std::endl;

    // std::cout << engine.m_board.fen() << std::endl;
    // chess::SearchOptions options;
    // options["movetime"] = 5000;

    // engine.m_main_thread.setup(engine.m_board, engine.m_search_cache, options.limits());
    // engine.m_main_thread.iterative_deepening();

    

    return 0;
}