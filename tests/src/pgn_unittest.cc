#include <gtest/gtest.h>
#include "includes.h"

TEST(PGN, pgn_game_status)
{
    using namespace chess;
    EXPECT_EQ(PGN::pgn_game_status(GameStatus::ONGOING, true), "*");
    EXPECT_EQ(PGN::pgn_game_status(GameStatus::CHECKMATE, true), "1-0");
    EXPECT_EQ(PGN::pgn_game_status(GameStatus::CHECKMATE, false), "0-1");
    EXPECT_EQ(PGN::pgn_game_status(GameStatus::DRAW, true), "1/2-1/2");
}


TEST(PGN, get_move_notation)
{
    using namespace chess;
    init();
    Board board("K6k/8/8/8/8/N7/8/4N3 w - - 0 1");
    Move move = board.match(Move::fromUci("e1c2"));
    std::string notation = PGN::get_move_notation(board, move);
    EXPECT_EQ(notation, "Nec2");
}