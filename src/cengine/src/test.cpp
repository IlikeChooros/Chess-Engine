#include <cengine/test.h>


namespace test
{

    // Got this from: 
    // https://gist.github.com/peterellisjones/8c46c28141c162d1d8a0f0badbc9cff9
    const PerftTestData::PerftData PerftTestData::data[] = {
            {1, 8, "r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2"},
            {1, 8, "8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3"},
            {1, 19, "r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2"},
            {1, 5, "r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2"},
            {1, 44, "2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2"},
            {1, 39, "rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9"},
            {1, 9, "2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4"},
            {3, 62379, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"},
            {3, 89890, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"},
            {6, 1134888, "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1"},
            {6, 1015133, "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1"},
            {6, 1440467, "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1"},
            {6, 661072, "5k2/8/8/8/8/8/8/4K2R w K - 0 1"},
            {6, 803711, "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"},
            {4, 1274206, "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1"},
            {4, 1720476, "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1"},
            {6, 3821001, "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"},
            {5, 1004658, "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1"},
            {6, 217342, "4k3/1P6/8/8/8/8/K7/8 w - - 0 1"},
            {6, 92683, "8/P1k5/K7/8/8/8/8/8 w - - 0 1"},
            {6, 2217, "K1k5/8/P7/8/8/8/8/8 w - - 0 1"},
            {7, 567584, "8/k1P5/8/1K6/8/8/8/8 w - - 0 1"},
            {4, 23527, "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1"}
    };

    Perft::Perft(Board* board)
    {
        m_expected = 0;
        m_print = true;
        m_time_us = 1;
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
    uint64_t Perft::run(int depth, std::string fen)
    {
        using namespace std::chrono;

        if (fen == Board().init().getFen() && depth >= 1 && depth <= perft_max_depth)
            m_expected = nodes_perft[depth - 1]; 
       
        m_board->loadFen(fen.c_str());
        ManagerImpl manager(m_board);
        auto start = high_resolution_clock::now();
        
        // auto ml = manager.move_list;
        // size_t moves = manager.generateMoves();

        MoveList ml;
        size_t moves = manager.gen_legal_moves(&ml);

        std::vector<uint64_t> nodes_path(moves, 1);
        
        
        if(depth == 1){
            m_time_us = (duration_cast<microseconds>(high_resolution_clock::now() - start)).count();
            printResults(1, moves, nodes_path.data(), manager);

            // if (new_nodes != size_t(manager.n_moves)){
            //     printf("New nodes: %lu, old nodes: %d\n", new_nodes, manager.n_moves);

            //     for (int i = 0; i < manager.n_moves; i++){
            //         bool found = false;
            //         for(size_t j = 0; j < new_nodes; j++){
            //             if (ml[j] == (uint32_t)manager.move_list[i]){
            //                 found = true;
            //                 break;
            //             }
            //         }

            //         if (!found){
            //             auto move = Move(manager.move_list[i]);
            //             int from = move.getFrom();
            //             int to = move.getTo();
            //             printf("Not found: %s\n", Piece::notation((*m_board)[from], to).c_str());
            //         }
            //     }
            // }            
            
            return moves;
        }

        uint64_t nodes = 0;
        for(size_t i = 0; i < moves; i++){
            auto move = Move(ml[i]);
            manager.make(move);
            nodes_path[i] = perft(depth - 1);
            nodes += nodes_path[i];
            manager.unmake();
        }

        m_time_us = (duration_cast<microseconds>(high_resolution_clock::now() - start)).count();
        printResults(depth, nodes, nodes_path.data(), manager);
        return nodes;
    }

    uint64_t Perft::perft(int depth)
    {
        ManagerImpl manager(m_board);

        // auto ml = manager.move_list;
        // size_t moves = manager.generateMoves();

        MoveList ml;
        size_t moves = manager.gen_legal_moves(&ml);

        if(depth == 1){
            return (uint64_t)moves;
        }

        uint64_t nodes = 0;
        for(size_t i = 0; i < moves; i++){
            auto move = Move(ml[i]);
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
        
        // for(int i = 0; i < manager.n_moves; i++){
        //     auto move = Move(manager.move_list[i]);
        //     int from = move.getFrom();
        //     int to = move.getTo();
        //     printf("Move: %s %lu\n", Piece::notation((*m_board)[from], to).c_str(), nodes_path[i]);
        // }
        printf("NPS: %lu\n", m_time_us != 0 ? (nodes * 1000000 / m_time_us) : 0);
        printf("Depth %d, Time: %lu us, Count: %lu", depth, m_time_us, nodes);
        if (m_expected != 0){
            printf(" %s %lu (expected)\n\n", nodes == m_expected ? "=" : "!=", m_expected);
            return;
        }
        printf("\n\n");
    }
}