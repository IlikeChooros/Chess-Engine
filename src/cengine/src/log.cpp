#include <cengine/log.h>

Log glogger = Log((global_settings.base_path / "log.txt").string());

Log::Log(std::string logfile)
{
    m_log_enabled = true;
    m_log_file    = logfile;
    m_log_stream.open(m_log_file, std::ios::out | std::ios::app);
}

Log::~Log()
{
    m_log_stream.close();
}

/**
 * @brief Log a string to the log file
 */
void Log::log(std::string& str)
{
    if (!m_log_enabled)
        return;
    
    m_log_stream << str;
    m_log_stream.flush();
}

/**
 * @brief Set the log file, if the filename is empty, logging is disabled
 */
void Log::setLogFile(std::string logfile)
{
    m_log_file = logfile;
    m_log_stream.close();
    m_log_enabled = logfile != ""; // Disable logging if the file is empty
    
    if (m_log_enabled)
        m_log_stream.open(m_log_file, std::ios::out | std::ios::app);
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
 * @brief Log transposition table information
 */
void Log::logTTableInfo(TTable<TEntry>* ttable)
{
    logf("Transposition Table Info: "
        "Size: %lu, ",
        ttable->getTable().size()
    );
}

/**
 * @brief Log board information
 */
void Log::logBoardInfo(chess::Board* board)
{
    logf("Board Info: "
        "FEN: %s, "
        "Side to move: %s, "
        "Fullmoves: %d, "
        "Halfmoves: %d, "
        "En passant: %s\n",
        board->fen().c_str(),
        board->getSide() == chess::Piece::White ? "White" : "Black",
        board->fullmoveCounter(),
        board->halfmoveClock(),
        chess::square_to_str(board->enpassantTarget()).c_str()
    );
}

/**
 * @brief Log the principal variation
 */
void Log::logPV(chess::MoveList* pv)
{
    logf("PV: ");
    for (auto& move : *pv)
    {
        logf("%s ", chess::Move(move).uci().c_str());
    }
    logf("\n");
}

/**
 * @brief Print search info
 */
void Log::printInfo(int depth, int score, bool cp, uint64_t nodes, uint64_t time, chess::MoveList* pv)
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
            str += chess::Move(move).uci() + " ";
        }
    }

    str += "\n";
    if (m_print_enabled)
        std::cout << str;

    log(str);
}

/**
 * @brief Print a formatted string
 */
void Log::printf(const char* str, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, str);
    vsnprintf(buffer, 1024, str, args);
    va_end(args);

    std::string s(buffer);
    if (m_print_enabled)
        std::cout << s;
        
    log(s);
}
