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

void Log::printToFile(std::string& str)
{
    m_log_stream << str;
    m_log_stream.flush();
}

void Log::print(const char* format, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
    va_end(args);
    
    // ::printf("%s", buffer);

    std::string str(buffer);
    printToFile(str);
}

void Log::printTTableInfo(TTable<TEntry>* ttable)
{
    print("Transposition Table Info: ");
    print("Size: %lu, ", ttable->getTable().size());
    print("Max size: %lu, ", ttable->getTable().max_size());
    print("Used size: %.5f%, ", float(ttable->getTable().size()) / ttable->getTable().max_size() * 100.0f);
    print("Load factor: %f, ", ttable->getTable().load_factor());
    print("Max load factor: %f, ", ttable->getTable().max_load_factor());
    print("Bucket count: %lu, ", ttable->getTable().bucket_count());
    print("Max bucket count: %lu\n", ttable->getTable().max_bucket_count());
}

void Log::printBoardInfo(chess::Board* board)
{
    print("Board Info: ");
    print("FEN: %s\n", board->getFen().c_str());
    print("Side to move: %s, ", board->getSide() == chess::Piece::White ? "White" : "Black");
    print("Fullmoves: %d, ", board->fullmoveCounter());
    print("Halfmoves: %d, ", board->halfmoveClock());
    print("En passant: %s\n", square_to_str(board->enpassantTarget()).c_str());
}

void Log::printPV(MoveList* pv)
{
    print("PV: ");
    for (auto& move : *pv)
    {
        print("%s ", chess::Piece::notation(Move(move).getFrom(), Move(move).getTo()).c_str());
    }
    print("\n");
}

void Log::printGameHistory(chess::GameHistory* gh)
{
    print("Game History: ");
    for (auto& hist : gh->history)
    {
        print("%s%c ", 
            chess::Piece::notation(Move(hist.move).getFrom(), Move(hist.move).getTo()).c_str(), 
            hist.side_to_move == chess::Piece::White ? 'W' : 'B'
        );
    }
    print("\n");
}

void Log::printStats(Move bestmove, int depth, int score, uint64_t nodes, uint64_t time)
{
    print("Stats: ");
    print("bestmove %s, ", chess::Piece::notation(bestmove.getFrom(), bestmove.getTo()).c_str());
    print("depth %d, ", depth);
    print("eval %d, ", score);
    print("nodes %lu, ", nodes);
    print("time %lu\n", time);
    time = std::max(time, (uint64_t)1);
    print("nps %lu\n", (nodes * 1000) / time);
}

