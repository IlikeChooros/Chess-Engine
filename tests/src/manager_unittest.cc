#include <gtest/gtest.h>
#include <algorithm>
#include "includes.h"


namespace{

using namespace chess;

// Fixture for playing moves with the manager
class ManagerTest : public ::testing::Test
{
    protected:
    Board board;
    Manager manager;

    void SetUp() override 
    {
        board.init();
        manager = Manager(&board);
        manager.init();
        manager.generateMoves();
    }

    // Load a fen string into the board and generate moves
    void loadFen(const char* fen)
    {
        manager.loadFen(fen);
    }

    // Check if a list of moves contains a move
    void containsMove(std::string from, std::string to, bool does_contain = true)
    {
        std::list<Manager::PieceMoveInfo> moves = manager.getPieceMoves(str_to_square(from));
        for(auto& move : moves){
            if(square_to_str(move.x + move.y * 8) == to){
                if(does_contain){
                    return;
                } else {
                    break;
                }
            }
        }
        if(does_contain){
            FAIL() << "*** Move from " << from << " to " << to << " should be present";
        }
    }

    // Check if a list of moves is equal to a list of expected moves
    void expectMoves(std::string from, std::list<std::string> expected)
    {
        std::list<Manager::PieceMoveInfo> moves = manager.getPieceMoves(str_to_square(from));
        std::list<std::string> moves_str;
        for(auto move : moves){
            moves_str.push_back(square_to_str(move.x + move.y * 8));
        }
        moves_str.sort();
        expected.sort();
        EXPECT_EQ(moves_str, expected);
    }

    // Move a piece from one square to another
    void move(std::string from, std::string to)
    {
        manager.makeMove(str_to_square(from), str_to_square(to));
    }
};


// Manager tests

TEST_F(ManagerTest, makeMove)
{
    manager.makeMove(str_to_square("e2"), str_to_square("e4"));

    EXPECT_EQ(board[str_to_square("e4")], Piece::createPiece(Piece::Pawn, Piece::White));
    EXPECT_EQ(board[str_to_square("e2")], 0);
}

TEST_F(ManagerTest, getPieceMoves)
{
    expectMoves("e2", {"e3", "e4"});
    std::list<Manager::PieceMoveInfo> moves = manager.getPieceMoves(str_to_square("e1"));
    EXPECT_EQ(moves.size(), 0);
}

TEST_F(ManagerTest, loadFenEnpassant)
{
    loadFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");

    EXPECT_EQ(board.enpassantTarget(), str_to_square("f6"));
    containsMove("e5", "f6");
}

TEST_F(ManagerTest, loadCheckmatePos)
{
    loadFen("6k1/ppp3Q1/2np4/2b1p2N/4P3/3P1P2/PPP1KP1P/8 b - - 1 27");

    // Check if the checkmate position is detected
    ASSERT_EQ(manager.impl()->n_moves, 0);
}

// Position tests
TEST_F(ManagerTest, checkBasicCastlingRights)
{
    loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    // Check if the castling moves are present
    containsMove("e1", "g1");
    containsMove("e1", "c1");
}

TEST_F(ManagerTest, checkCastlingRightsAfterMove)
{
    loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    manager.makeMove(str_to_square("e1"), str_to_square("e2"));
    manager.makeMove(str_to_square("e8"), str_to_square("e7"));

    manager.makeMove(str_to_square("e2"), str_to_square("e1"));
    manager.makeMove(str_to_square("e7"), str_to_square("e8"));

    manager.generateMoves();

    // Check if the castling moves aren't present (there shouldn't be any)
    containsMove("e1", "g1", false);
    containsMove("e1", "c1", false);

    manager.makeMove(str_to_square("e1"), str_to_square("e2"));

    containsMove("e8", "g8", false);
    containsMove("e8", "c8", false);
}

// Test move generation

// Test special pawn cases, when enpassant is possible and the rook is pinning the pawn
TEST_F(ManagerTest, testMoveGenEnpassantPin)
{
    loadFen("8/8/8/r2Pp2K/8/8/k7/8 w - e6 0 1");
    // Can't enpassant because of pin
    containsMove("d5", "e6", false);
}

TEST_F(ManagerTest, testMoveGenEnpassantPinNotPinned)
{
    loadFen("8/8/8/r2Pp1PK/8/8/k7/8 w - e6 0 1");
    // Enpassant should be possible, there is a pawn near the king
    containsMove("d5", "e6");
}

TEST_F(ManagerTest, testMoveGenCheckmate)
{
    loadFen("rnbqkbnr/ppppp2p/5p2/6pQ/3PP3/8/PPP2PPP/RNB1KBNR b KQkq - 1 3");
    // Checkmate position, so there should be no moves
    ASSERT_EQ(manager.impl()->n_moves, 0);
}

TEST_F(ManagerTest, testMoveGenCapturePromotion)
{
    loadFen("rnbqkbnr/pP3ppp/8/8/4p3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 5");

    // Check if the promotion capture move is present
    containsMove("b7", "a8", true);
    containsMove("b7", "c8", true);

    // Check if the correct flags are set
    auto vflags = manager.getFlags(str_to_square("b7"), str_to_square("a8"));
    ASSERT_EQ(vflags.size(), 4);
    for(int i = 0; i < 4; i++){
        vflags[i] <<= 12;
        ASSERT_TRUE(Move(vflags[i]).isPromotionCapture());
    }
}

TEST_F(ManagerTest, testMoveGenPromotion)
{
    loadFen("r1bqkbnr/pP3ppp/2n5/8/4pP2/8/PPPP2PP/RNBQKBNR w KQkq - 1 6");

    // Check if the promotion move is present
    containsMove("b7", "b8", true);

    // Check if the correct flags are set
    auto vflags = manager.getFlags(str_to_square("b7"), str_to_square("b8"));
    ASSERT_EQ(vflags.size(), 4);
    for(int i = 0; i < 4; i++){
        vflags[i] <<= 12;
        Move m(vflags[i]);
        ASSERT_TRUE(m.isPromotion());
        ASSERT_FALSE(m.isPromotionCapture());
    }
}


TEST_F(ManagerTest, testPreft)
{
    bench::Perft perft;
    constexpr uint64_t nodes_perft[6] = {20, 400, 8902, 197281, 4865609, 119060324};
    ASSERT_EQ(perft.run(6), nodes_perft[6 - 1]);
}

} // namespace