#include <cengine/test.h>


namespace test
{

    Perft::Perft(Board* board)
    {
        m_print = true;
        m_time_ms = 0;
        m_board = board;
    }

    Perft& Perft::operator=(Perft&& other)
    {
        m_board = other.m_board;
        return *this;
    }

    /**
     * @brief Run the perft test at the specified depth >= 1
     */
    uint64_t Perft::run(int depth)
    {
        using namespace std::chrono;
        
        m_init_pos = m_board->getFen() == Board().init().getFen();
        ManagerImpl manager(m_board);
        auto start = high_resolution_clock::now();

        manager.generateMoves();
        std::vector<uint64_t> nodes_path(manager.n_moves, 1);
        
        if(depth == 1){
            printResults(1, (uint64_t)manager.n_moves, nodes_path.data(), manager);
            return (uint64_t)manager.n_moves;
        }

        uint64_t nodes = 0;
        for(int i = 0; i < manager.n_moves; i++){
            auto move = Move(manager.move_list[i]);
            manager.make(move);
            nodes_path[i] = perft(depth - 1);
            nodes += nodes_path[i];
            manager.unmake();
        }

        m_time_ms = (duration_cast<milliseconds>(high_resolution_clock::now() - start)).count();
        printResults(depth, nodes, nodes_path.data(), manager);
        return nodes;
    }

    uint64_t Perft::perft(int depth)
    {
        ManagerImpl manager(m_board);

        manager.generateMoves();

        if(depth == 1){
            return (uint64_t)manager.n_moves;
        }

        uint64_t nodes = 0;
        for(int i = 0; i < manager.n_moves; i++){
            auto move = Move(manager.move_list[i]);
            manager.make(move);
            nodes += perft(depth - 1);
            manager.unmake();
        }
        
        return nodes;
    }

    void Perft::printResults(int depth, uint64_t nodes, const uint64_t* nodes_path, ManagerImpl &manager)
    {
        if(!m_print)
            return;
        
        for(int i = 0; i < manager.n_moves; i++){
            auto move = Move(manager.move_list[i]);
            int from = move.getFrom();
            int to = move.getTo();
            printf("Move: %s %lu\n", Piece::notation((*m_board)[from], to).c_str(), nodes_path[i]);
        }
        printf("Depth %d, Time: %lu ms, Count: %lu", depth, m_time_ms, nodes);
        if (depth >= perft_max_depth || !m_init_pos){
            printf("\n\n");
            return;
        }
        printf(" %s %lu (expected)\n\n", nodes == nodes_perft[depth-1] ? "=" : "!=", nodes_perft[depth-1]);
    }
}