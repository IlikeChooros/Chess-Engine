#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    uci::UCI uci;
    uci.loop();

    // chess::init();
    // chess::Engine engine;
    // engine.setPosition("r5k1/2b2ppp/4pq2/3pN3/3P4/2PBP3/PPQ2PPP/R5K1 w - - 0 1");

    // std::cout << engine.m_board.fen() << std::endl;
    // chess::SearchOptions options;
    // options["movetime"] = 5000;

    // engine.m_main_thread.setup(engine.m_board, engine.m_search_cache, options.limits());
    // engine.m_main_thread.iterative_deepening();

    // std::cout << chess::Eval::evaluate(engine.m_board) * (engine.m_board.turn() ? 1 : -1) << std::endl;

    return 0;
}