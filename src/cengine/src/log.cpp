#include <cengine/log.h>

Log glogger = Log((global_settings.base_path / "log.txt").string());

Log::Log(std::string logfile)
{
    m_log_file = logfile;
    m_log_stream.open(m_log_file, std::ios::out | std::ios::app);
}

Log::~Log()
{
    m_log_stream.close();
}

void Log::log(std::string& str)
{
    m_log_stream << str;
    m_log_stream.flush();
}

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
        square_to_str(board->enpassantTarget()).c_str()
    );
}

void Log::logPV(chess::MoveList* pv)
{
    logf("PV: ");
    for (auto& move : *pv)
    {
        logf("%s ", chess::Move(move).uci().c_str());
    }
    logf("\n");
}

void Log::logGameHistory(chess::GameHistory* gh)
{
    logf("Game History: ");
    for (auto& hist : gh->history)
    {
        logf("%s%c ", 
            chess::Move(hist.move).uci().c_str(),
            hist.side_to_move == chess::Piece::White ? 'W' : 'B'
        );
    }
    logf("\n");
}

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
    std::cout << str;
    log(str);
}

void Log::printf(const char* str, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, str);
    vsnprintf(buffer, 1024, str, args);
    va_end(args);

    std::string s(buffer);
    std::cout << s;
    log(s);
}
