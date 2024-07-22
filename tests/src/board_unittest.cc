#include <gtest/gtest.h>
#include <cengine/board.h>

namespace{
    
bool compareBoards(int piece, int expected, int index){
    return piece == expected;
}

TEST(Board, loadFen){
    using namespace chess;
    Board board;
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    board.loadFen(fen);
    int expected_board[64] = {0};
    expected_board[0] = Piece::getCastleRook(Piece::Black);
    expected_board[1] = Piece::Knight | Piece::Black;
    expected_board[2] = Piece::Bishop | Piece::Black;
    expected_board[3] = Piece::Queen | Piece::Black;
    expected_board[4] = Piece::getCastleKing(Piece::Black);
    expected_board[5] = Piece::Bishop | Piece::Black;
    expected_board[6] = Piece::Knight | Piece::Black;
    expected_board[7] = Piece::getCastleRook(Piece::Black);

    for(int i = 0; i < 8; i++){
        expected_board[i + 8] = Piece::createPiece(Piece::Pawn, Piece::Black);
        expected_board[i + 48] = Piece::createPiece(Piece::Pawn, Piece::White);
        expected_board[i + 16] = expected_board[i + 24] = expected_board[i + 32] = expected_board[i + 40] = 0;
    }

    expected_board[56] = Piece::getCastleRook(Piece::White);
    expected_board[57] = Piece::Knight | Piece::White;
    expected_board[58] = Piece::Bishop | Piece::White;
    expected_board[59] = Piece::Queen | Piece::White;
    expected_board[60] = Piece::getCastleKing(Piece::White);
    expected_board[61] = Piece::Bishop | Piece::White;
    expected_board[62] = Piece::Knight | Piece::White;
    expected_board[63] = Piece::getCastleRook(Piece::White);

    for(int i = 0; i < 64; i++){
        EXPECT_PRED3(compareBoards, board[i], expected_board[i], i);
    }

    EXPECT_EQ(board.getSide(), Piece::White);
    EXPECT_EQ(board.enpassantTarget(), -1);
    EXPECT_EQ(board.halfmoveClock(), 0);
    EXPECT_EQ(board.fullmoveCounter(), 1);
}
} // namespace