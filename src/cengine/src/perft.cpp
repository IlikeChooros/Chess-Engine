#include <cengine/perft.h>


namespace bench
{

Perft::Perft(bool print)
{
    m_print = print;
}

Perft& Perft::operator=(Perft&& other)
{
    m_print = other.m_print;
    return *this;
}

/**
 * @brief Run the perft test at the specified depth >= 1
 * @return Total number of nodes
 */
uint64_t Perft::run(int depth, std::string fen)
{
    if (depth < 1)
        return 0;

    // Load the fen, TODO: add `moves` support
    m_board.loadFen(fen.c_str());
    uint64_t total = perft<true>(depth);

    if(m_print) {
        std::cout<< "Nodes: " << total << "\n\n";
    }
    return total;
}

/**
 * @brief Actual perft implementation, with root flag, to print the results
 */
template <bool root>
uint64_t Perft::perft(int depth)
{
    chess::MoveList moves = m_board.generateLegalMoves();

    if(depth == 1){
        if (root && m_print) 
        {
            for(size_t i = 0; i < moves.size(); i++)
            {
                std::cout << moves[i].uci() << ": " << 1UL << "\n";
            }
        }  
        return (uint64_t)moves.size();
    }

    uint64_t nodes = 0, cnodes = 0;
    for(size_t i = 0; i < moves.size(); i++)
    {
        auto move = moves[i];
        m_board.makeMove(move);
        cnodes = perft<false>(depth - 1);
        nodes += cnodes;
        m_board.undoMove(move);

        if (root && m_print)
        {
            std::cout << move.uci() << ": " << cnodes << "\n";
        }
    }
    
    return nodes;
}

} // namespace bench