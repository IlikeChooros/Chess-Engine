#include <gtest/gtest.h>
#include <algorithm>
#include "includes.h"


namespace{

using namespace chess;

// Fixture for playing moves with the manager
class ManagerTest : public ::testing::Test{
    protected:
    Board board;
    Manager manager;

    void SetUp() override {
        board.init();
        manager = Manager(&board);
    }

    // Load a fen string into the board and generate moves
    void loadFen(const char* fen){
        board.loadFen(fen);
        manager.generateMoves();
    }

    // Check if a list of moves contains a move
    void containsMove(std::string from, std::string to, bool does_contain = true){
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
            FAIL() << "Move from " << from << " to " << to << " should be present";
        }
    }

    // Check if a list of moves is equal to a list of expected moves
    void expectMoves(std::string from, std::list<std::string> expected){
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
    void move(std::string from, std::string to){
        manager.movePiece(str_to_square(from), str_to_square(to));
    }
};


// Manager tests

TEST_F(ManagerTest, movePiece){
    manager.movePiece(str_to_square("e2"), str_to_square("e4"));

    EXPECT_EQ(board[str_to_square("e4")], Piece::createPiece(Piece::Pawn, Piece::White));
    EXPECT_EQ(board[str_to_square("e2")], 0);
}

TEST_F(ManagerTest, getPieceMoves){
    expectMoves("e2", {"e3", "e4"});
    std::list<Manager::PieceMoveInfo> moves = manager.getPieceMoves(str_to_square("e1"));
    EXPECT_EQ(moves.size(), 0);
}

TEST_F(ManagerTest, loadFenEnpassant){
    loadFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");

    EXPECT_EQ(board.enpassantTarget(), str_to_square("f6"));
    containsMove("e5", "f6");
}

TEST_F(ManagerTest, loadCheckmatePos){
    loadFen("6k1/ppp3Q1/2np4/2b1p2N/4P3/3P1P2/PPP1KP1P/8 b - - 1 27");

    // int n_moves = manager.generateMoves();
    // EXPECT_EQ(n_moves, 0);
}

// Position tests
TEST_F(ManagerTest, checkBasicCastlingRights){
    loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    // Check if the castling moves are present
    containsMove("e1", "g1");
    containsMove("e1", "c1");
    containsMove("e8", "g8");
    containsMove("e8", "c8");
}

TEST_F(ManagerTest, checkCastlingRightsAfterMove){
    loadFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");

    manager.movePiece(str_to_square("e1"), str_to_square("e2"));
    manager.movePiece(str_to_square("e8"), str_to_square("e7"));

    manager.movePiece(str_to_square("e2"), str_to_square("e1"));
    manager.movePiece(str_to_square("e7"), str_to_square("e8"));

    manager.generateMoves();

    // Check if the castling moves aren't present (there shouldn't be any)
    containsMove("e1", "g1", false);
    containsMove("e1", "c1", false);
    containsMove("e8", "g8", false);
    containsMove("e8", "c8", false);
}

// Test move generation
TEST_F(ManagerTest, testEnpassantPin){
    loadFen("8/8/8/r2Pp2K/8/8/k7/8 w - e6 0 1");
    // Can't enpassant because of pin
    containsMove("e5", "e6", false);
}



} // namespace