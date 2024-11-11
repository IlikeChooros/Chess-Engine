#include <cengine/engine.h>

namespace chess
{

Engine::Engine()
{
    m_board.init();
}

uint64_t Engine::perft(int depth)
{
    return bench::Perft().run(depth, m_board.getFen());
}

Move Engine::go(SearchOptions& options)
{
    return Move();
}

void Engine::setPosition(const std::string& fen)
{
    m_board.loadFen(fen);
}

}// namespace chess