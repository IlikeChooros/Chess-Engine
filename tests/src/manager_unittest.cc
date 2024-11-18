#include <gtest/gtest.h>
#include <algorithm>
#include "includes.h"


namespace{

using namespace chess;

// Fixture for playing moves with the manager
class ManagerTest : public ::testing::Test
{
    protected:
    chess::Board board;

    void SetUp() override 
    {
        chess::init();
        board.init();
    }

    // Load a fen string into the board and generate moves
    void loadFen(const char* fen)
    {
        board.loadFen(fen);
    }

    // Actual join function implementation
    std::string join(std::vector<std::string> v, std::string delim)
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

    // Check if a list of moves contains a move
    void containsMove(std::string uci_move, bool should_be_present = true)
    {
        bool found = false;
        auto moves = board.generateLegalMoves();
        for(auto& move : moves)
        {
            if(chess::Move(move).uci() == uci_move)
            {
                found = true;
                break;
            }
        }

        if (should_be_present != found)
        {
            std::string str_moves = join(moves.uci(), ", ");
            FAIL() << "*** Move " << uci_move << (should_be_present ? " not found" : " found") 
                    << " in moves: " << str_moves;
        }
    }

    /**
     * @brief Get the moves from a square
     */
    MoveList getMovesFrom(std::string from)
    {
        Square from_sq = str_to_square(from);
        return board.filterMoves([from_sq](Move move){
            return move.getFrom() == uint32_t(from_sq);
        });
    }

    // Check if a list of moves is equal to a list of expected moves
    void expectMoves(std::string from, std::list<std::string> expected)
    {
        auto moves_str = getMovesFrom(from).uci();
        std::list<std::string> moves_str_list(moves_str.begin(), moves_str.end());

        moves_str_list.sort();
        expected.sort();
        EXPECT_EQ(moves_str_list, expected);
    }

    // Move a piece from one square to another
    void move(std::string move)
    {
        board.makeMove(board.match(Move::fromUci(move)));
    }
};


// Manager tests

TEST_F(ManagerTest, makeMove)
{
    move("e2e4");
    EXPECT_EQ(board[str_to_square("e4")], Piece::createPiece(Piece::Pawn, Piece::White));
    EXPECT_EQ(board[str_to_square("e2")], Piece::Empty);
}

TEST_F(ManagerTest, getPieceMoves)
{
    expectMoves("e2", {"e2e3", "e2e4"});
    auto moves = getMovesFrom("e1");
    EXPECT_EQ(moves.size(), 0);
}

TEST_F(ManagerTest, loadFenEnpassant)
{
    loadFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    EXPECT_EQ(board.enpassantTarget(), str_to_square("f6"));
    containsMove("e5f6");
}

TEST_F(ManagerTest, loadCheckmatePos)
{
    loadFen("6k1/ppp3Q1/2np4/2b1p2N/4P3/3P1P2/PPP1KP1P/8 b - - 1 27");
    // Check if the checkmate position is detected
    ASSERT_EQ(board.generateLegalMoves().size(), 0);
}

// Position tests
TEST_F(ManagerTest, checkBasicCastlingRights)
{
    loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    // Check if the castling moves are present
    containsMove("e1g1");
    containsMove("e1c1");
}

TEST_F(ManagerTest, checkCastlingRightsAfterMove)
{
    loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    move("e1e2");
    move("e8e7");
    move("e2e1");
    move("e7e8");

    // Check if the castling moves aren't present (there shouldn't be any)
    containsMove("e1g1", false);
    containsMove("e1c1", false);

    move("e1e2");

    containsMove("e8g8", false);
    containsMove("e8c8", false);
}

// Test move generation

// Test special pawn cases, when enpassant is possible and the rook is pinning the pawn
TEST_F(ManagerTest, testMoveGenEnpassantPin)
{
    loadFen("8/8/8/r2Pp2K/8/8/k7/8 w - e6 0 1");
    // Can't enpassant because of pin
    containsMove("d5e6", false);
}

TEST_F(ManagerTest, testMoveGenEnpassantPinNotPinned)
{
    loadFen("8/8/8/r2Pp1PK/8/8/k7/8 w - e6 0 1");
    // Enpassant should be possible, there is a pawn near the king
    containsMove("d5e6");
}

TEST_F(ManagerTest, testMoveGenCheckmate)
{
    loadFen("rnbqkbnr/ppppp2p/5p2/6pQ/3PP3/8/PPP2PPP/RNB1KBNR b KQkq - 1 3");
    // Checkmate position, so there should be no moves
    ASSERT_EQ(board.generateLegalMoves().size(), 0);
}

TEST_F(ManagerTest, testMoveGenCapturePromotion)
{
    loadFen("rnbqkbnr/pP3ppp/8/8/4p3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 5");

    // Check if the promotion capture move is present
    containsMove("b7a8q", true);
    containsMove("b7a8r", true);
    containsMove("b7a8b", true);
    containsMove("b7a8n", true);

    containsMove("b7c8q", true);
    containsMove("b7c8r", true);
    containsMove("b7c8b", true);
    containsMove("b7c8n", true);
}

TEST_F(ManagerTest, testMoveGenPromotion)
{
    loadFen("r1bqkbnr/pP3ppp/2n5/8/4pP2/8/PPPP2PP/RNBQKBNR w KQkq - 1 6");

    // Check if the promotion move is present
    containsMove("b7b8q", true);

    auto move = board.match(Move::fromUci("b7b8q"));

    ASSERT_TRUE(move.isPromotion());
    ASSERT_FALSE(move.isCapture());
}


TEST_F(ManagerTest, testPreft)
{
    bench::Perft perft;
    constexpr uint64_t nodes_perft[6] = {20, 400, 8902, 197281, 4865609, 119060324};
    ASSERT_EQ(perft.run(6), nodes_perft[6 - 1]);
}

} // namespace