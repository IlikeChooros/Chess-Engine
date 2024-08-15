#pragma once

#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <stdarg.h>

#include "move.h"
#include "board.h"
#include "transp_table.h"
#include "settings.h"
#include "history.h"

class Log
{
    std::string m_log_file;
    std::ofstream m_log_stream;

    void printToFile(std::string& str);
public:
    Log(std::string logfile = "log.txt");
    ~Log();

    void print(const char* format, ...);
    void printTTableInfo(TTable<TEntry>* ttable);
    void printBoardInfo(chess::Board* board);
    void printPV(MoveList* pv);
    void printGameHistory(chess::GameHistory* gh);
    void printStats(Move bestmove, int depth, int score, uint64_t nodes, uint64_t time);
};

extern Log glogger;