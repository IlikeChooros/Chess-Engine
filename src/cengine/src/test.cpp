#include <cengine/test.h>


namespace test
{

    // Source (added some cases):
    // https://gist.github.com/peterellisjones/8c46c28141c162d1d8a0f0badbc9cff9
    const PerftTestData::PerftData PerftTestData::data[] = {
            {1, 8, "r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2"},
            {1, 8, "8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3"},
            {1, 19, "r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2"},
            {1, 5, "r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2"},
            {1, 44, "2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2"},
            {1, 39, "rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9"},
            {1, 9, "2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4"},
            {1, 6, "3k4/8/8/K1Pp3r/8/8/8/8 w - d6 0 2"},
            {1, 24, "rn1qkbnr/ppp1pppp/8/8/4p1b1/5Q2/PPPP1PPP/RNBK1BNR w kq - 2 4"},
            {1, 6, "6k1/5pp1/1Q2b2p/4P3/7P/8/3r2PK/3q4 w - - 1 34 moves b6b4 d1e2 b4f4 e2e5"},
            {1, 24, "r4rk1/pppb1p2/3bq2p/3NN2Q/2B3p1/8/PP1R2PP/4R2K b - - 0 23 moves d7c8 d5b6 f7f6 h5e8"},
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
            {4, 23527, "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1"},
    };

    Perft::Perft(Board* board)
    {
        m_print = true;
        m_board = board;
    }

    Perft& Perft::operator=(Perft&& other)
    {
        m_print = other.m_print;
        m_board = other.m_board;
        return *this;
    }

    /**
     * @brief Run the perft test at the specified depth >= 1
     */
    uint64_t Perft::run(int depth, std::string fen)
    {       
        m_board->loadFen(fen.c_str());
        uint64_t total = perft<true>(depth);
        if(m_print) {
            for(auto line : m_results)
                std::cout << line << "\n";
            std::cout<< "Nodes: " << total;
        }
            
        return total;
    }

    template <bool root>
    uint64_t Perft::perft(int depth)
    {
        ManagerImpl manager(m_board);
        size_t moves = manager.generateMoves();
        auto ml = manager.move_list;

        if(depth == 1){
            if (root && m_print) {
                m_results.reserve(moves);
                for(size_t i = 0; i < moves; i++)
                    m_results.push_back(
                        Piece::notation(ml[i].getFrom(), ml[i].getTo()) + ": " + std::to_string(1UL)
                    );
            }       // printf("%s: %lu\n", Piece::notation(Move(ml[i]).getFrom(), Move(ml[i]).getTo()).c_str(), 1UL);
            return (uint64_t)moves;
        }

        uint64_t nodes = 0, cnodes = 0;
        for(size_t i = 0; i < moves; i++){
            auto move = Move(ml[i]);
            manager.make(move);
            cnodes = perft<false>(depth - 1);
            nodes += cnodes;
            manager.unmake();
            if (root && m_print)
                m_results.push_back(
                    Piece::notation(move.getFrom(), move.getTo()) + ": " + std::to_string(cnodes)
                );
                // printf("%s: %lu\n", Piece::notation(move.getFrom(), move.getTo()).c_str(), cnodes);
        }
        
        return nodes;
    }


    void TestPerftStockfish::run(int depth, std::string fen, std::string stockfish)
    {
        Perft p(m_board);
        p.run(depth, fen);
        p.setPrint(true);
        auto results = p.getResults();

        std::cout << "\n\n";

        for (auto res : results){
            // Find given move in stockfish results, if not found, print it
            bool found = false;
            std::istringstream iss(stockfish);
            std::string line;
            while (std::getline(iss, line)){
                if (line.find(res) != std::string::npos){
                    found = true;
                    break;
                }
            }

            if (!found){
                std::cout << "Not found " << res << std::endl;
            }
        }
    }


    const std::string openings[] = {
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2", // e4 e5
        "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2", // e4 d5
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2", // e4 c5
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2", // e4 c5 Nf3
        "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2",
        "rnb1kbnr/ppp1pppp/8/3q4/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3",
        "rnb1kbnr/ppp1pppp/8/3q4/3P4/8/PPP2PPP/RNBQKBNR b KQkq d3 0 3",
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
        "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
        "rnbqkbnr/pppp1ppp/8/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR b KQkq - 1 2",
        "rnbqkbnr/pp2pppp/3p4/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3",
        "rnbqkbnr/pp2pppp/3p4/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3",
        "rnbqkb1r/pp2pppp/3p1n2/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R w KQkq - 1 4",
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2",
        "rnbqkbnr/ppp1pppp/8/3p4/3P1B2/8/PPP1PPPP/RN1QKBNR b KQkq - 1 2", // london
        "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
        "rnbqkb1r/ppp1pppp/5n2/3p4/2PP4/5N2/PP2PPPP/RNBQKB1R b KQkq c3 0 3",
        "rnbqkbnr/ppp2ppp/4p3/3P4/3P4/8/PP2PPPP/RNBQKBNR b KQkq - 0 3",
        "rnbqkb1r/pppp1ppp/4pn2/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 1 3",
        "rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    };


    TestGamePlay::GameData TestGamePlay::data[TestGamePlay::n_games] = {};

    void TestGamePlay::run()
    {
        using namespace std::chrono;
        {
            ManagerImpl m;
            m.init();
        }
        for (int i = 0; i < n_games; i++){
            
            high_resolution_clock::time_point t1 = high_resolution_clock::now();
            Board b;
            b.loadFen(openings[i % (sizeof(openings) / sizeof(openings[0]))].c_str());
            ManagerImpl m(&b);
            m.generateMoves();

            // Simulate game
            GameStatus status = GameStatus::ONGOING;
            while(status == ONGOING){
                auto result = m.search();
                status = m.getStatus();
                m.make(result.move);
                m.generateMoves();
            }
            
            // Save game data
            data[i] = {
                openings[i % (sizeof(openings) / sizeof(openings[0]))],
                b.getFen(),
                m.getStatus(),
                duration_cast<seconds>(high_resolution_clock::now() - t1).count(),
                b.fullmoveCounter(),
                status == CHECKMATE ? b.getSide() == Piece::White ? -1 : 1 : 0
            };

            // Print game data
            std::cout << "\nGame " << i + 1 << "\n";
            print(data[i]);
        }
    }

    void TestGamePlay::print(GameData &data)
    {
        std::cout << "Start: " << data.start_pos << "\n";
        std::cout << "End: " << data.end_pos << "\n";
        std::cout << "Result: " << data.result << "\n";
        std::cout << "Time: " << data.time << "\n";
        std::cout << "Moves: " << data.moves << "\n";
        std::cout << "Color win: " << data.colorWin << "\n";
    }

    void TestGamePlay::printResults()
    {
        for (int i = 0; i < n_games; i++){
            std::cout << "\nGame " << i + 1 << "\n";
            print(data[i]);
        }
    }
}