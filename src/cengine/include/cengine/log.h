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

// Logging class, every log message is written to a file
class Log
{
    bool m_log_enabled;
    std::string m_log_file;
    std::ofstream m_log_stream;

    void log(std::string& str);
public:
    static constexpr const char* LOG_FILE = "log.txt";

    Log(std::string logfile = LOG_FILE);
    ~Log();

    void setLogFile(std::string logfile);
    void logf(const char* format, ...);
    void logTTableInfo(TTable<TEntry>* ttable);
    void logBoardInfo(chess::Board* board);
    void logPV(chess::MoveList* pv);
    void printInfo(int depth, int score, bool cp, uint64_t nodes, uint64_t time, chess::MoveList* pv = nullptr);
    void printf(const char* format, ...);
};

extern Log glogger;