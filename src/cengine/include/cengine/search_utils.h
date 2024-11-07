#pragma once

#include <mutex>
#include <list>
#include <map>

#include "move.h"
#include "eval.h"

namespace chess
{

constexpr int MIN = -(1 << 29),
             MAX = 1 << 29,
             MATE = -(1 << 28) + 1,
             MATE_THRESHOLD = -MATE;

// Score struct
struct Score{

    // Type of score (either centipawns or mate)
    enum Type{
        cp, mate
    } type = cp;

    // Value of the score
    int value = 0;
};

// Updates the score based on the given evaluation
inline void update_score(Score& score, int eval, int whotomove, MoveList& pv)
{
    // Update the score
    score.type = Score::cp;
    score.value = eval * whotomove;
    
    // If that's a mate score
    if (abs(eval) >= MATE_THRESHOLD)
    {
        score.value = (pv.size() + 1) / 2;
        score.value = std::max(score.value, 1);
        score.value *= eval > 0 ? 1 : -1;
        score.value *= whotomove;
        score.type = Score::mate;
    }
}

// Search result
// move: Best move found
// score: Score of the position (type, value)
// depth: Depth of the search
// time: Time taken to search (in milliseconds)
// status: Game status (ongoing, checkmate, stalemate, draw)
// mutex: Mutex to protect the result
// pv: Principal variation
struct SearchResult
{
    Move move = Move(Move::nullMove);
    Score score = {Score::cp, 0};
    int depth = 0;
    uint64_t time = 0;
    GameStatus status = ONGOING;
    std::mutex mutex = {};
    std::list<Move> pv = {};
};

// Updates the search result with thread safety
inline void update_result(SearchResult* result, Move move, Score score, int depth, uint64_t time, GameStatus status, MoveList& pv)
{
    std::unique_lock<std::mutex> lock(result->mutex);
    if (lock.owns_lock())
    {
        result->move = move;
        result->score = score;
        result->depth = depth;
        result->time = time;
        result->status = status;
        result->pv = std::list<Move>(pv.begin(), pv.end());
    }
}


struct SearchParams
{
    // Basic UCI options
    int depth = INT32_MAX; // In plies, that is max possible depth
    uint64_t nodes = UINT64_MAX;
    int mate = 0;
    int64_t movetime = INT64_MAX; // In milliseconds
    uint64_t wtime = 0;
    uint64_t btime = 0;
    uint64_t winc = 0;
    uint64_t binc = 0;
    bool infinite = false;
    bool ponder = false;

    // Asynchronous search
    bool stop = false;
    bool is_running = false;
    uint64_t nodes_searched = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::mutex mutex;

    SearchParams() = default;

    SearchParams(const SearchParams& other)
    {
        *this = other;
    }

    SearchParams& operator=(const SearchParams& other)
    {
        depth = other.depth;
        movetime = other.movetime;
        nodes = other.nodes;
        movetime = other.movetime;
        infinite = other.infinite;
        ponder = other.ponder;
        stop = other.stop;
        return *this;
    }

    void setSearchRunning(bool running)
    {
        std::lock_guard<std::mutex> lock(mutex);
        is_running = running;
    }

    void setSearchStop(bool stop)
    {
        std::lock_guard<std::mutex> lock(mutex);
        this->stop = stop;
    }

    void stopSearch()
    {
        setSearchStop(true);
    }

    bool shouldStop()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return stop;
    }

    void resetStop()
    {
        setSearchStop(false);
    }
};

// Main object to handle search options
class SearchOptions
{
public:

    // Search option
    class Option
    {
    public:
        typedef int value_t;
        typedef std::function<void(SearchParams&, value_t value)> Setter;

        // Default setters
        static void defaultSetter(SearchParams&, value_t) {}
        static void setDepth(SearchParams& params, value_t value) { params.depth = value; }
        static void setNodes(SearchParams& params, value_t value) { params.nodes = value; }
        static void setInfinite(SearchParams& params, value_t value) { params.infinite = bool(value); }
        static void setPonder(SearchParams& params, value_t value) { params.ponder = bool(value); }
        static void setMovetime(SearchParams& params, value_t value) { params.movetime = value; }
        static void setWtime(SearchParams& params, value_t value) { params.wtime = value; }
        static void setBtime(SearchParams& params, value_t value) { params.btime = value; }

        // Constructors
        Option(Setter setter = defaultSetter) : m_setter(setter), m_params(nullptr) {}
        Option(Setter setter, SearchParams* param): m_setter(setter), m_params(param) {}
        Option(const Option& other): m_setter(other.m_setter), m_params(other.m_params) {}

        Option& operator=(const Option& other) 
        { 
            m_setter = other.m_setter; 
            return *this; 
        }

        Option& operator=(Setter setter) { m_setter = setter; return *this; }
        void operator=(value_t value) const 
        { 
            if(m_params)
                m_setter(*m_params, value); 
        }
        
        // Invoke the setter
        void operator()(value_t value) const
        {
            if (m_params)
                m_setter(*m_params, value);
        }

    private:
        Setter m_setter;
        SearchParams *m_params;
    };

    SearchOptions()
    {
        m_options["depth"] = Option(Option::setDepth, &m_params);
        m_options["nodes"] = Option(Option::setNodes, &m_params);
        m_options["infinite"] = Option(Option::setInfinite, &m_params);
        m_options["ponder"] = Option(Option::setPonder, &m_params);
        m_options["movetime"] = Option(Option::setMovetime, &m_params);
        m_options["wtime"] = Option(Option::setWtime, &m_params);
        m_options["btime"] = Option(Option::setBtime, &m_params);
    }

    void setOption(const std::string& name, Option::value_t value)
    {
        auto it = m_options.find(name);
        if (it != m_options.end())
            it->second(value);
    }

    Option& operator[](const std::string& name)
    {
        return m_options[name];
    }

private:
    SearchParams m_params;
    std::map<std::string, Option> m_options;
};

} // namespace chess