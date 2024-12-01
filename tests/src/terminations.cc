#include <gtest/gtest.h>
#include "includes.h"   

namespace
{

using namespace chess;

class TerminationTest: public ::testing::Test
{
protected:
    Board board;

    void SetUp() override
    {
        chess::init();
        board.init();
    }

    void position(std::string fen)
    {
        board.loadFen(fen);
    }
};


TEST_F(TerminationTest, checkmate)
{
    position("8/8/8/8/8/4k3/8/r3K3 w - - 6 4");
    EXPECT_EQ(board.isTerminated(), true);
    EXPECT_EQ(board.getTermination(), Termination::CHECKMATE);
}


TEST_F(TerminationTest, stalemate)
{
    position("4k3/4P3/4K3/8/8/8/8/8 b - - 0 1");
    EXPECT_EQ(board.isTerminated(), true);
    EXPECT_EQ(board.getTermination(), Termination::STALEMATE);
}

TEST_F(TerminationTest, fiftyMoves)
{
    position("7k/ppp5/8/8/8/8/7K/8 w - - 100 1");
    EXPECT_EQ(board.isTerminated(), true);
    EXPECT_EQ(board.getTermination(), Termination::FIFTY_MOVES);
}

TEST_F(TerminationTest, insufficientMaterial)
{
    constexpr const char* FENS[] = {
        "4k3/8/8/5KB1/8/8/8/8 w - - 0 1",
        "4k3/8/8/5K2/8/8/8/8 w - - 0 1",
        "4k3/8/8/5KN1/8/8/8/8 w - - 0 1",
        "4kn2/8/8/5KB1/8/8/8/8 w - - 0 1",
        "4kb2/8/8/5KB1/8/8/8/8 w - - 0 1",
    };

    for (const auto& fen : FENS)
    {
        position(fen);
        EXPECT_EQ(board.isTerminated(), true) << "FEN: " << fen;
        EXPECT_EQ(board.getTermination(), Termination::INSUFFICIENT_MATERIAL);
    }
}

TEST_F(TerminationTest, threefoldRepetition)
{
    constexpr const char* FENS[] = {
        "8/8/8/r7/8/7K/2k5/8 w - - 0 1 moves h3g3 a5a4 g3h3 a4a5 h3g3 a5a4 g3h3 a4a5",
        "8/8/8/r7/8/7K/2k5/8 b - - 0 1 moves a5a4 h3g3 a4a5 g3h3 a5a4 h3g3 a4a5 g3h3",
        "8/5pk1/6p1/8/4Q3/8/5K2/8 w - - 0 1 moves e4e5 g7g8 e5e8 g8g7 e8e5 g7g8 e5e8 g8g7 e8e5",
        "8/6k1/8/8/4QR2/8/5K2/8 w - - 0 1 moves f4h4 g7g8 h4f4 g8g7 f4h4 g7g8 h4f4 g8g7",
        "8/6k1/5qr1/8/8/8/6K1/8 w - - 0 1 moves g2h3 g6h6 h3g2 h6g6 g2h3 g6h6 h3g2 h6g6",
    };

    for (const auto& fen : FENS)
    {
        position(fen);
        EXPECT_EQ(board.isTerminated(), true) << "FEN: " << fen << " " << board.history().size();
        EXPECT_EQ(board.getTermination(), Termination::THREEFOLD_REPETITION);
    }
}

} // namespace