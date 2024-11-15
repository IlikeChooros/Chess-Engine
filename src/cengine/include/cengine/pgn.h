#pragma once

#include "eval.h"
#include "board.h"
#include "history.h"
#include "manager.h"

// TODO: remove mangager from the pgn, use only the board

struct PGNFields
{
    std::string Event = "";
    std::string Site = "";
    std::chrono::system_clock::time_point Date = std::chrono::system_clock::now();
    int Round = 1;
    std::string White = "";
    std::string Black = "";
    struct Result{
        chess::GameStatus status = chess::GameStatus::ONGOING;
        int colorWin = chess::Piece::White;
    } Result;
    std::string FEN = "";
};

class PGN
{
public:
    static std::string pgn(chess::GameHistory* gh, PGNFields& fields);
    static std::string pgn_game_status(chess::GameStatus status, bool white = true);
    static std::string get_move_notation(chess::Manager* m, chess::GameHistory *gh, chess::Move move);
};