#include <cengine/engine.h>

namespace chess
{

/**
 * @brief Initialize the boards, search and evaluation,
 * same as `chess::init()`
 */
void Engine::init()
{
    init_board();
    init_eval();
    init_hashing();
    init_magics(false);
}

Engine::Engine()
{
    m_board.init();
}

Engine::~Engine()
{
    m_main_thread.join();
}

/**
 * @brief Move constructor, threads are not copied
 */
Engine& Engine::operator=(Engine&& other)
{
    m_board = std::move(other.m_board);
    m_search_cache = std::move(other.m_search_cache);
    return *this;
}

/**
 * @brief Reset the search cache (transposition table) as
 * 'ucinewgame' command expects
 */
void Engine::reset()
{
    m_search_cache.getTT().clear();
    m_search_cache.getHH().clear();
}

/**
 * @brief Run a perft test at the specified depth
 * @param depth Depth of the perft test
 * @param print Whether to print the results
 * @return Total number of nodes
 */
uint64_t Engine::perft(int depth, bool print)
{
    return bench::Perft().run(depth, m_board.getFen());
}

/**
 * @brief Start the search in async mode, call `join()` to make this function synchronous
 * @return Future with `Result` object, has the best move found, score and other info
 */
std::future<Result> Engine::go(SearchOptions& options)
{
    m_main_thread.join();
    m_main_thread.start_thinking(m_board, m_search_cache, options.limits());
    return m_main_thread.get_future();
}

/**
 * @brief Wait for the search to finish
 */
void Engine::join()
{
    m_main_thread.join();
}

/**
 * @brief Stop the search if it is running and join the thread
 */
void Engine::stop()
{
    m_main_thread.stop();
}

/**
 * @brief Set the position of the board, using FEN notation
 * @param fen FEN string, has uci full support ('startpos', FEN, FEN + moves)
 */
void Engine::setPosition(const std::string& fen)
{
    std::string FEN = fen == "startpos" ? std::string(Board::START_FEN) : fen;
    m_board.loadFen(FEN);
}

/**
 * @brief Set the position of the board
 */
void Engine::setPosition(const Board& board)
{
    m_board = board;
}

}// namespace chess