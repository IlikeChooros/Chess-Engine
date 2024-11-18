#include <gtest/gtest.h>
#include "includes.h"

namespace
{

using namespace chess;

class HashTest : public ::testing::Test
{
    protected:
    Board board;

    void SetUp() override 
    {
        chess::init();
        board.init();
    }

    // Actual join function implementation
    std::string join(std::vector<std::string> v, std::string delim = ", ")
    {
        std::string str;
        for (size_t i = 0; i < v.size(); i++)
        {
            str += v[i];
            if (i != v.size() - 1)
                str += delim;
        }
        return str;
    }
};


// Test quiet move doesn't change position flags, just piece positions
TEST_F(HashTest, QuietMakeMoveUpdate)
{
    board.makeMove(board.match(Move("g1f3")));
    auto hash = board.m_hash;

    // .hash() will generate the hash of the board
    EXPECT_EQ(hash, board.hash());
}

// Test quite move, but for black
TEST_F(HashTest, QuietMakeMoveUpdateBlack)
{
    board.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    board.makeMove(board.match(Move("g8f6")));
    auto hash = board.m_hash;

    // .hash() will generate the hash of the board
    EXPECT_EQ(hash, board.hash());
}

// Test double move, should update enpassant hash, hash modification, by `makeMove`
TEST_F(HashTest, DoubleMoveMakeMoveUpdate)
{
    board.makeMove(board.match(Move("e2e4")));
    auto hash = board.m_hash; // Updated by `makeMove`

    // .hash() will generate the hash of the board
    EXPECT_EQ(hash, board.hash());
}

// Test double move, but for black
TEST_F(HashTest, DoubleMoveMakeMoveUpdateBlack)
{
    board.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    board.makeMove(board.match(Move("e7e5")));
    auto hash = board.m_hash; // Updated by `makeMove`

    // .hash() will generate the hash of the board
    EXPECT_EQ(hash, board.hash());
}

// Test capture pawn move
TEST_F(HashTest, CaptureMakeMoveUpdate)
{
    board.loadFen("rnbqkbnr/ppp1p1pp/5p2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3");
    board.makeMove(board.match(Move("d5e6")));
    auto hash = board.m_hash; // Updated by `makeMove`

    // .hash() will generate the hash of the board
    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test enpassant capture move
TEST_F(HashTest, EnpassantCaptureMakeMoveUpdate)
{
    board.loadFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    board.makeMove(board.match(Move("e5f6")));
    auto hash = board.m_hash; // Updated by `makeMove`

    // .hash() will generate the hash of the board
    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test promotion move
TEST_F(HashTest, PromotionMakeMoveUpdate)
{
    board.loadFen("r1bqkbnr/pP3ppp/8/8/4p3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 5");
    board.makeMove(board.match(Move("b7b8q")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test promotion capture move
TEST_F(HashTest, PromotionCaptureMakeMoveUpdate)
{
    board.loadFen("rnbqkbnr/pP3ppp/8/8/4p3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 5");
    board.makeMove(board.match(Move("b7a8q")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test castling move
TEST_F(HashTest, CastlingMakeMoveUpdate)
{
    board.loadFen("r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
    board.makeMove(board.match(Move("e1g1")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test long castling move
TEST_F(HashTest, LongCastlingMakeMoveUpdate)
{
    board.loadFen("r3kbnr/pppb1ppp/2np1q2/4p1B1/4P3/2NP1Q2/PPP2PPP/2KR1BNR b kq - 3 6");
    std::cout << "side: " << board.turn() << "\n";
    auto moves = join(board.generateLegalMoves().uci());

    board.makeMove(board.match(Move::fromUci("e8c8")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()) << "Moves: " << moves << "\n";
}

// Test losing castling rights by moving the king
// losing castling rights is already tested in `manager_unittest.cc`
TEST_F(HashTest, LoseCastlingRights)
{
    board.loadFen("r3kbnr/pppb1ppp/2np1q2/4p1B1/4P3/2NP1Q2/PPP2PPP/2KR1BNR b kq - 3 6");
    board.makeMove(board.match(Move("e8e7")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test losing castling rights by moving the king
// losing castling rights is already tested in `manager_unittest.cc`
TEST_F(HashTest, QuietMove)
{
    board.loadFen("r3kbnr/pppb1ppp/2np1q2/4p1B1/4P3/2NP1Q2/PPP2PPP/2KR1BNR b kq - 3 6");
    board.makeMove(board.match(Move("b7b6")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

// Test losing castling rights by moving the rook,
TEST_F(HashTest, LoseCastlingRightsRook)
{
    board.loadFen("r3kbnr/pppb1ppp/2np1q2/4p1B1/4P3/2NP1Q2/PPP2PPP/2KR1BNR b kq - 3 6");
    board.makeMove(board.match(Move("a8b8")));
    auto hash = board.m_hash; // Updated by `makeMove`

    EXPECT_EQ(hash, board.hash()); // Hash should be the same
}

} // namespace