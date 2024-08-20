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
public:
    enum LogFlags
    {
        file = 1 << 0,
        console = 1 << 1,
    };

    typedef struct 
    {
        int flags = file | console;
    } LogSettings;

    Log(std::string logfile = "log.txt");
    ~Log();

    void logf(const char* format, ...);
    void logTTableInfo(TTable<TEntry>* ttable);
    void logBoardInfo(chess::Board* board);
    void logPV(MoveList* pv);
    void logGameHistory(chess::GameHistory* gh);
    void printInfo(int depth, int score, bool cp, uint64_t nodes, uint64_t time, MoveList* pv = nullptr, bool log = true);
    void printf(const char* format, ...);

    /**
     * @brief Set the log settings
     */
    inline void setSettings(LogSettings settings) { m_settings = settings; }

    /**
     * @brief Get the log flags
     */
    inline void setf(int flags) { m_settings.flags = flags; }

private:
    LogSettings m_settings;
    std::string m_log_file;
    std::ofstream m_log_stream;

    void log(std::string& str);
    void sprint(std::string& str);
};

extern Log glogger;