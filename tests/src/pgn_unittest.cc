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
    Board board;
    board.loadFen("K6k/8/8/8/8/N7/8/4N3 w - - 0 1");
    Manager manager(&board);
    manager.init();
    manager.generateMoves();

    manager.makeMove(str_to_square("e1"), str_to_square("c2"));
    GameHistory gh = manager.impl()->history;
    manager.unmake();
    manager.generateMoves();
    
    auto notation = PGN::get_move_notation(&manager, &gh, gh.history[1].move);
    EXPECT_EQ(notation, "Nec2 ");
}