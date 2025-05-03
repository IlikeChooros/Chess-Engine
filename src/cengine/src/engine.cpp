#include <cengine/engine.h>

namespace chess
{

/**
 * @brief Initialize the boards, search and evaluation,
 * same as `chess::init()`
 */
void Engine::base_init()
{
    Board::init_board();
    Eval::init();
    init_hashing();
    init_magics(false);
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
    m_search_cache.getKH().clear();
}

/**
 * @brief Run a perft test at the specified depth
 * @param depth Depth of the perft test
 * @param print Whether to print the results
 * @return Total number of nodes
 */
uint64_t Engine::perft(int depth, bool print)
{
    return bench::Perft(print).run(depth, m_board.fen());
}

/**
 * @brief Start the search in async mode, call `join()` to make this function synchronous
 * @return Atomic like result object, use `get()` to get the result
 */
shared_data<Result>& Engine::go(const SearchOptions& options)
{
    m_main_thread.stop();
    m_main_thread.start_thinking(m_board, m_search_cache, options.limits());
    return m_main_thread.get_result();
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
bool Engine::setPosition(const std::string& fen)
{
    return m_board.loadFen(fen);
}

/**
 * @brief Set the position of the board, but in a stream
 * @param fen stream with the FEN string, will stop parsing when the fen becomes invalid (or reads the whole fen section)
 */
bool Engine::setPosition(std::istringstream& fen)
{
    return m_board.loadFen(fen);
}

/**
 * @brief Set the position of the board
 */
void Engine::setPosition(const Board& board)
{
    m_board = board;
}

// UCI options

/**
 * @brief Set the hash size
 * @param hash Size of the hash table in MB
 */
void Engine::setHashSize(size_t size)
{
    m_search_cache.getTT() = TTable<TEntry>(size);
}

/**
 * @brief Set the log file
 * @param file Path to the log file, if empty no log will be written
 */
void Engine::setLogFile(const std::string& file)
{
    glogger.setLogFile(file);
}

}// namespace chess