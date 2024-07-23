#include <gtest/gtest.h>
#include <algorithm>
#include "includes.h"


namespace{

using namespace chess;

// Check if a list of moves contains a move
void containsMove(std::list<Manager::PieceMoveInfo> moves, std::string expected){
    std::list<std::string> moves_str;
    for(auto move : moves){
        moves_str.push_back(square_to_str(move.x + move.y * 8));
    }
    EXPECT_TRUE(std::find(moves_str.begin(), moves_str.end(), expected) != moves_str.end());
}

// Check if a list of moves is equal to a list of expected moves
void expectMoves(std::list<Manager::PieceMoveInfo> moves, std::list<std::string> expected){
    std::list<std::string> moves_str;
    for(auto move : moves){
        moves_str.push_back(square_to_str(move.x + move.y * 8));
    }
    moves_str.sort();
    expected.sort();
    EXPECT_EQ(moves_str, expected);
}

// Manager tests

TEST(Manager, movePiece){
    Board board;
    board.init();
    Manager manager(&board);
    manager.movePiece(str_to_square("e2"), str_to_square("e4"));

    EXPECT_EQ(board[str_to_square("e4")], Piece::createPiece(Piece::Pawn, Piece::White));
    EXPECT_EQ(board[str_to_square("e2")], 0);
}

TEST(Manager, getPieceMoves){
    Board board;
    board.init();
    Manager manager(&board);

    expectMoves(manager.getPieceMoves(str_to_square("e2")), {"e3", "e4"});
    std::list<Manager::PieceMoveInfo> moves = manager.getPieceMoves(str_to_square("e1"));
    EXPECT_EQ(moves.size(), 0);
}

TEST(Manager, loadFenEnpassant){
    Board board;
    board.loadFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");    
    Manager manager(&board);

    EXPECT_EQ(board.enpassantTarget(), str_to_square("f6"));
    containsMove(manager.getPieceMoves(str_to_square("e5")), "f6");
}

TEST(Manager, loadCheckmatePos){
    Board board;
    board.loadFen("6k1/ppp3Q1/2np4/2b1p2N/4P3/3P1P2/PPP1KP1P/8 b - - 1 27");
    Manager manager(&board);

    // int n_moves = manager.generateMoves();
    // EXPECT_EQ(n_moves, 0);
}

} // namespace