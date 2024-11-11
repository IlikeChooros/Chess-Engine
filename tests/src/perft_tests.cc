#include <gtest/gtest.h>
#include <cengine/cengine.h>

namespace 
{
    class PerftTest: public testing::Test
    {
    protected:
        bench::Perft perft;

        void SetUp() override 
        {
            chess::init();
            perft = bench::Perft();
        }
    };

    
    TEST_F(PerftTest, PerftTest)
    {
        for (size_t i = 0; i < 23; i++)
        {
            auto t = test::PerftTestData::data[i];
            printf("Running test %zu...", i + 1);
            uint64_t nodes = perft.run(t.depth, t.fen);
            ASSERT_EQ(nodes, t.nodes) << " for given FEN = " << t.fen 
                                         << " and depth = " << t.depth 
                                         << " nodes are invalid";
        }
    }

}