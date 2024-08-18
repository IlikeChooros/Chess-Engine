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

    void log(std::string& str);
public:
    Log(std::string logfile = "log.txt");
    ~Log();

    std::string logf(const char* format, ...);
    void logTTableInfo(TTable<TEntry>* ttable);
    void logBoardInfo(chess::Board* board);
    void logPV(MoveList* pv);
    void logGameHistory(chess::GameHistory* gh);
    void printInfo(Move bestmove, int depth, int score, bool cp, uint64_t nodes, uint64_t time, MoveList *pv = nullptr);
    void printf(const char* format, ...);
};

extern Log glogger;