#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    uci::UCI uci;
    uci.loop(argc, argv);

    // chess::init();
    // chess::Engine engine;

    // const char* positions[] = {
    //     "2rqkb1r/1p1npppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQ1RK1 w k - 1 9",
    //     "1rb3k1/1p3qpp/p2b4/3N4/2P5/1P4P1/P2Q1RBP/6K1 b - - 4 28",
    //     "rnbqkb1r/ppp2ppp/4pn2/3p4/2PP4/4PN2/PP3PPP/RNBQKB1R b KQkq - 0 4",
    // };

    // chess::SearchOptions options;
    // options["movetime"] = 5000;

    // for (size_t i = 0; i < sizeof(positions) / sizeof(positions[0]); i++)
    // {
    //     engine.setPosition(positions[i]);
    //     engine.m_main_thread.setup(engine.m_board, engine.m_search_cache, options.limits());
    //     engine.m_main_thread.iterative_deepening();
    // }

    // using namespace chess;
    // chess::Engine engine;
    // engine.init();
    // chess::Board& board = engine.board();

    // board.loadFen("2rqkb1r/1p1npppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQ1RK1 w k - 1 9");
    // board.generateLegalMoves();
    // std::cout << Eval::evaluate(board) << "\n";

    // board.loadFen("r2qkb1r/pp1npppp/3p1n2/2p5/4P3/2N2N2/PPPP1PPP/R1BQ1RK1 b kq - 3 6");
    // board.generateLegalMoves();
    // std::cout << Eval::evaluate(board) << "\n";

    // board.loadFen("4k3/6pp/5p2/8/6P1/5PKP/8/8 w - - 0 1");
    // board.generateLegalMoves();
    // std::cout << Eval::evaluate(board) << "\n";

    return 0;
}