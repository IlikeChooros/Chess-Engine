#include <cengine/test.h>


namespace test
{

    // Source (added some cases):
    // https://gist.github.com/peterellisjones/8c46c28141c162d1d8a0f0badbc9cff9
    const PerftTestData::PerftData PerftTestData::data[] = 
    {
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
}