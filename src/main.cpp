#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    // uci::UCI uci;
    // uci.loop();

    chess::init();
    chess::Engine engine;
    engine.setPosition("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2 moves h7h4");
    chess::SearchOptions options;
    options["movetime"] = 5000;

    engine.m_main_thread.setup(engine.m_board, engine.m_search_cache, options.limits());
    engine.m_main_thread.iterative_deepening();

    // chess::init();
    // chess::Engine engine;
    // engine.setPosition("r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2 moves h7h4");
    // auto moves = engine.m_board.generateLegalMoves();
    // chess::MoveOrdering::sort(&moves, nullptr, &engine.m_board, &engine.m_search_cache);

    return 0;
}