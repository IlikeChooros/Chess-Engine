#include <gtest/gtest.h>
#include "includes.h"

namespace{

using namespace chess;

bool compareBoards(int piece, int expected, int index)
{
    return piece == expected;
}

void testFen(
    const char* fen, int expected_board[64], 
    int expected_side, int expected_enpassant, 
    int expected_halfmove, int expected_fullmove
)
{
    Board board;
    board.loadFen(fen);
    for(int i = 0; i < 64; i++){
        EXPECT_PRED3(compareBoards, board[i], expected_board[i], i);
    }
    EXPECT_EQ(board.getSide(), expected_side);
    EXPECT_EQ(board.enpassantTarget(), expected_enpassant);
    EXPECT_EQ(board.halfmoveClock(), expected_halfmove);
    EXPECT_EQ(board.fullmoveCounter(), expected_fullmove);
}

TEST(Board, loadFen){

    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int expected_board[64] = {0};
    expected_board[0] = Piece::getRook(Piece::Black);
    expected_board[1] = Piece::Knight | Piece::Black;
    expected_board[2] = Piece::Bishop | Piece::Black;
    expected_board[3] = Piece::Queen | Piece::Black;
    expected_board[4] = Piece::getKing(Piece::Black);
    expected_board[5] = Piece::Bishop | Piece::Black;
    expected_board[6] = Piece::Knight | Piece::Black;
    expected_board[7] = Piece::getRook(Piece::Black);

    for(int i = 0; i < 8; i++){
        expected_board[i + 8] = Piece::createPiece(Piece::Pawn, Piece::Black);
        expected_board[i + 48] = Piece::createPiece(Piece::Pawn, Piece::White);
        expected_board[i + 16] = expected_board[i + 24] = expected_board[i + 32] = expected_board[i + 40] = 0;
    }

    expected_board[56] = Piece::getRook(Piece::White);
    expected_board[57] = Piece::Knight | Piece::White;
    expected_board[58] = Piece::Bishop | Piece::White;
    expected_board[59] = Piece::Queen | Piece::White;
    expected_board[60] = Piece::getKing(Piece::White);
    expected_board[61] = Piece::Bishop | Piece::White;
    expected_board[62] = Piece::Knight | Piece::White;
    expected_board[63] = Piece::getRook(Piece::White);

    testFen(fen, expected_board, Piece::White, 0, 0, 1);
}

TEST(Board, loadFenKingPawnPos){
    const char* fen = "8/8/8/4p1K1/2k1P3/8/8/8 b - - 0 1";
    int expected_board[64] = {0};

    expected_board[str_to_square("c4")] = Piece::King | Piece::Black;
    expected_board[str_to_square("e5")] = Piece::Pawn | Piece::Black;
    expected_board[str_to_square("e4")] = Piece::Pawn | Piece::White;
    expected_board[str_to_square("g5")] = Piece::King | Piece::White;

    testFen(fen, expected_board, Piece::Black, 0, 0, 1);
}

TEST(Board, loadFenDrawPosition){
    const char* fen = "8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50";
    int expected_board[64] = {0};

    expected_board[str_to_square("f7")] = Piece::King | Piece::Black;
    expected_board[str_to_square("a4")] = Piece::Pawn | Piece::Black;
    expected_board[str_to_square("b5")] = Piece::Pawn | Piece::Black;
    expected_board[str_to_square("d6")] = Piece::Pawn | Piece::Black;
    expected_board[str_to_square("e5")] = Piece::Pawn | Piece::Black;
    expected_board[str_to_square("f4")] = Piece::Pawn | Piece::Black;
    expected_board[str_to_square("h5")] = Piece::Pawn | Piece::Black;

    expected_board[str_to_square("h3")] = Piece::King | Piece::White;
    expected_board[str_to_square("a3")] = Piece::Pawn | Piece::White;
    expected_board[str_to_square("b4")] = Piece::Pawn | Piece::White;
    expected_board[str_to_square("d5")] = Piece::Pawn | Piece::White;
    expected_board[str_to_square("e4")] = Piece::Pawn | Piece::White;
    expected_board[str_to_square("f3")] = Piece::Pawn | Piece::White;
    expected_board[str_to_square("h4")] = Piece::Pawn | Piece::White;

    testFen(fen, expected_board, Piece::Black, 0, 99, 50);
}

TEST(Board, getFen)
{
    Board b;
    b.init();
    std::string fen = b.fen();
    const char* target = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    EXPECT_STREQ(fen.c_str(), target);
}

TEST(Board, loadGetFen){
    Board b;
    const char* fen = "8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50";
    b.loadFen(fen);
    std::string getfen = b.fen();
    EXPECT_STREQ(fen, getfen.c_str());
}

} // namespace