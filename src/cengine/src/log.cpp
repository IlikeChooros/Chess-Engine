#include <cengine/log.h>

Log glogger = Log((global_settings.base_path / "log.txt").string());

Log::Log(std::string logfile)
{
    m_settings = LogSettings();
    m_log_file = logfile;
    m_log_stream.open(m_log_file, std::ios::out | std::ios::app);
}

Log::~Log()
{
    m_log_stream.close();
}

/**
 * @brief Log to file
 */
void Log::log(std::string& str)
{
    if (!(m_settings.flags & LogFlags::file))
        return;
    
    m_log_stream << str;
    m_log_stream.flush();
}

/**
 * @brief Print to console and log file
 */
void Log::sprint(std::string& str)
{
    if (m_settings.flags & LogFlags::console)
        std::cout << str;
    log(str);
}

/**
 * @brief Log a formatted string
 */
void Log::logf(const char* format, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
    va_end(args);

    std::string str(buffer);
    log(str);
}

/**
 * @brief Log transposition table info
 */
void Log::logTTableInfo(TTable<TEntry>* ttable)
{
    logf("Transposition Table Info: "
        "Size: %lu, "
        "Load factor: %f, "
        "Max load factor: %f, "
        "Bucket count: %lu, "
        "Max bucket count: %lu\n",
        ttable->getTable().size(),
        ttable->getTable().load_factor(),
        ttable->getTable().max_load_factor(),
        ttable->getTable().bucket_count(),
        ttable->getTable().max_bucket_count()
    );
}

/**
 * @brief Log board info
 */
void Log::logBoardInfo(chess::Board* board)
{
    logf("Board Info: "
        "FEN: %s, "
        "Side to move: %s, "
        "Fullmoves: %d, "
        "Halfmoves: %d, "
        "En passant: %s\n",
        board->getFen().c_str(),
        board->getSide() == chess::Piece::White ? "White" : "Black",
        board->fullmoveCounter(),
        board->halfmoveClock(),
        square_to_str(board->enpassantTarget()).c_str()
    );
}

/**
 * @brief Log principal variation
 */
void Log::logPV(MoveList* pv)
{
    logf("PV: ");
    for (auto& move : *pv)
    {
        logf("%s ", Move(move).notation().c_str());
    }
    logf("\n");
}

/**
 * @brief Log game history
 */
void Log::logGameHistory(chess::GameHistory* gh)
{
    logf("Game History: ");
    for (auto& hist : gh->history)
    {
        logf("%s%c ", 
            Move(hist.move).notation().c_str(),
            hist.side_to_move == chess::Piece::White ? 'W' : 'B'
        );
    }
    logf("\n");
}

/**
 * @brief Print search info (depth, score, nodes, time, pv) and log to file
 */
void Log::printInfo(int depth, int score, bool cp, uint64_t nodes, uint64_t time, MoveList* pv, bool log)
{
    time = std::max(time, 1UL);

    std::string str = 
        "info depth " + std::to_string(depth) 
        + " score " + (cp ? "cp " : "mate ") + std::to_string(score) 
        + " nodes " + std::to_string(nodes) 
        + " time " + std::to_string(time) 
        + " nps " + std::to_string(nodes * 1000 / time);


    if (pv && pv->size() > 0)
    {
        str += " pv ";
        for (auto& move : *pv)
        {
            str += Move(move).notation() + " ";
        }
    }

    str += "\n";
    this->sprint(str);
}

/**
 * @brief Log a formatted string to console and file
 */
void Log::printf(const char* str, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, str);
    vsnprintf(buffer, 1024, str, args);
    va_end(args);

    std::string s(buffer);
    this->sprint(s);
}
