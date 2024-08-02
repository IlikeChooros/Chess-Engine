#include <gtest/gtest.h>
#include <cengine/cengine.h>

namespace 
{
    class PerftTest: public testing::Test{
        protected:
        test::Perft perft;
        chess::Board board;

        void SetUp() override 
        {
            board.init();
            perft = test::Perft(&board);
        }

        void loadTest(const test::PerftTestData::PerftData& data)
        {
            board.loadFen(data.fen.c_str());
        }

        void runTest(const test::PerftTestData::PerftData& data)
        {
            loadTest(data);
            perft.setExpected(data.nodes);
            uint64_t nodes = perft.run(data.depth);
            ASSERT_EQ(nodes, data.nodes) << " for given FEN = " << data.fen 
                                         << " and depth = " << data.depth 
                                         << " nodes are invalid";
        }
    };

    
    TEST_F(PerftTest, PerftTest)
    {
        for (size_t i = 0; i < 6; i++)
        {
            printf("Running test %zu...", i + 1);
            runTest(test::PerftTestData::data[i]);
        }
    }

}