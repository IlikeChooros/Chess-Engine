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

std::string Log::logf(const char* format, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
    va_end(args);

    std::string str(buffer);
    log(str);
    return str;
}

void Log::logTTableInfo(TTable<TEntry>* ttable)
{
    (void)logf("Transposition Table Info: ");
    (void)logf("Size: %lu, ", ttable->getTable().size());
    (void)logf("Max size: %lu, ", ttable->getTable().max_size());
    (void)logf("Used size: %.5f%, ", float(ttable->getTable().size()) / ttable->getTable().max_size() * 100.0f);
    (void)logf("Load factor: %f, ", ttable->getTable().load_factor());
    (void)logf("Max load factor: %f, ", ttable->getTable().max_load_factor());
    (void)logf("Bucket count: %lu, ", ttable->getTable().bucket_count());
    (void)logf("Max bucket count: %lu\n", ttable->getTable().max_bucket_count());
}

void Log::logBoardInfo(chess::Board* board)
{
    (void)logf("Board Info: ");
    (void)logf("FEN: %s\n", board->getFen().c_str());
    (void)logf("Side to move: %s, ", board->getSide() == chess::Piece::White ? "White" : "Black");
    (void)logf("Fullmoves: %d, ", board->fullmoveCounter());
    (void)logf("Halfmoves: %d, ", board->halfmoveClock());
    (void)logf("En passant: %s\n", square_to_str(board->enpassantTarget()).c_str());
}

void Log::logPV(MoveList* pv)
{
    (void)logf("PV: ");
    for (auto& move : *pv)
    {
        (void)logf("%s ", chess::Piece::notation(Move(move).getFrom(), Move(move).getTo()).c_str());
    }
    (void)logf("\n");
}

void Log::logGameHistory(chess::GameHistory* gh)
{
    (void)logf("Game History: ");
    for (auto& hist : gh->history)
    {
        (void)logf("%s%c ", 
            chess::Piece::notation(Move(hist.move).getFrom(), Move(hist.move).getTo()).c_str(), 
            hist.side_to_move == chess::Piece::White ? 'W' : 'B'
        );
    }
    (void)logf("\n");
}

void Log::printInfo(Move bestmove, int depth, int score, bool cp, uint64_t nodes, uint64_t time, MoveList *pv)
{
    time = std::max<decltype(time)>(time, 1UL);
    auto output = logf("info currmove %s depth %d score %s %d nodes %lu time %lu nps %lu", 
        chess::Piece::notation(bestmove.getFrom(), bestmove.getTo()).c_str(), 
        depth, 
        cp ? "cp" : "mate", 
        score, 
        nodes, 
        time,
        nodes * 1000 / time
    );

    if (pv && pv->size() > 0)
    {
        output += logf(" pv ");
        for (auto& move : *pv)
        {
            output += logf("%s ", chess::Piece::notation(Move(move).getFrom(), Move(move).getTo()).c_str());
        }
    }
    output += logf("\n");
    
    std::cout << output;
}

void Log::printf(const char* format, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
    va_end(args);

    std::string str(buffer);
    log(str);

    std::cout << str;
}
