#pragma once

#include <mutex>
#include <list>
#include <map>

#include "move.h"
#include "eval.h"
#include "time_man.h"

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
    Value value = 0;
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


// Search result
struct Result
{
    Move bestmove = Move::nullMove;
    Score score = {Score::cp, 0};
    MoveList pv = {};
    GameStatus status = ONGOING;
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

// Contains search limits
typedef struct
{
    // In plies
    int depth = std::numeric_limits<int>::max();
    // Maximum number of nodes to search
    uint64_t nodes = std::numeric_limits<uint64_t>::max();
    // Mate in moves
    int mate = 0;

    // Time limits
    chess::TimeLimits time = {};

    // Pondering
    bool ponder = false;
} Limits;

// Class to handle interrupts (`stop` signal, time limit, etc.)
class Interrupt
{
    std::atomic_bool m_stop;
    std::atomic<uint64_t> m_nodes;
    Limits m_limits;
    chess::TimeMan m_time;
public:

    Interrupt() : 
        m_stop(false), m_nodes(0), 
        m_limits(), m_time() {}

    Interrupt(const Limits& limits) :
        m_stop(false), m_nodes(0), 
        m_limits(limits), m_time(limits.time) {}

    Interrupt(Interrupt&& other) 
    {
        *this = std::move(other);
    }

    Interrupt& operator=(Interrupt&& other)
    {
        m_stop   = other.m_stop.load();
        m_limits = other.m_limits;
        m_nodes  = other.m_nodes.load();
        m_time   = other.m_time;
        return *this;
    }

    // Set the stop signal
    void stop() { m_stop = true; }

    // Check if the stop signal is set
    bool get() const
    {
        return m_stop.load();
    }

    // Update depth
    void depth(int depth)
    {
        m_stop = (m_stop.load() || (depth > m_limits.depth));
    }

    // Increment nodes by 1, and check if the search should stop (based on time)
    void update(bool side)
    {
        add_nodes(1);
        m_time.check(side);
        m_stop = (m_stop.load() || m_time.get());
    }

    // Update nodes
    void add_nodes(uint64_t nodes)
    {
        m_nodes += nodes;
        m_stop = (m_stop.load() || (m_nodes.load() > m_limits.nodes));
    }

    // Get the number of nodes searched
    uint64_t nodes() const
    {
        return m_nodes.load();
    }

    // Get elapsed time
    chess::TimeMan::Time time() const
    {
        return m_time.elapsed();
    }
};

// Contains uci search limits
struct SearchLimits
{
    // Basic UCI options
    int depth = INT32_MAX; // In plies, that is max possible depth
    uint64_t nodes = UINT64_MAX;
    int mate = 0;
    int64_t movetime = INT64_MAX; // In milliseconds
    uint64_t time[2] = {0, 0};
    uint64_t inc[2] = {0, 0};
    bool infinite = false;
    bool ponder = false;

    // Asynchronous search
    bool stop = false;
    bool is_running = false;
    uint64_t nodes_searched = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::mutex mutex;

    SearchLimits() = default;

    SearchLimits(const SearchLimits& other)
    {
        *this = other;
    }

    SearchLimits& operator=(const SearchLimits& other)
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
        typedef std::function<void(SearchLimits&, value_t value)> Setter;

        // Default setters
        static void defaultSetter(SearchLimits&, value_t) {}
        static void setDepth(SearchLimits& params, value_t value) { params.depth = value; }
        static void setNodes(SearchLimits& params, value_t value) { params.nodes = value; }
        static void setInfinite(SearchLimits& params, value_t value) { params.infinite = bool(value); }
        static void setPonder(SearchLimits& params, value_t value) { params.ponder = bool(value); }
        static void setMovetime(SearchLimits& params, value_t value) { params.movetime = value; }
        static void setWtime(SearchLimits& params, value_t value) { params.time[1] = value; }
        static void setBtime(SearchLimits& params, value_t value) { params.time[0] = value; }
        static void setWinc(SearchLimits& params, value_t value) { params.inc[1] = value; }
        static void setBinc(SearchLimits& params, value_t value) { params.inc[0] = value; }

        // Constructors
        Option(Setter setter = defaultSetter) : m_setter(setter), m_limits(nullptr) {}
        Option(Setter setter, SearchLimits* param): m_setter(setter), m_limits(param) {}
        Option(const Option& other): m_setter(other.m_setter), m_limits(other.m_limits) {}

        Option& operator=(const Option& other) 
        { 
            m_limits = other.m_limits;
            m_setter = other.m_setter; 
            return *this; 
        }

        // Set the setter
        Option& operator=(Setter setter) { m_setter = setter; return *this; }

        // Calls the setter for given value
        void operator=(value_t value) const 
        { 
            if(m_limits)
                m_setter(*m_limits, value); 
        }
        
        // Invoke the setter
        void operator()(value_t value) const
        {
            if (m_limits)
                m_setter(*m_limits, value);
        }

    private:
        Setter m_setter;
        SearchLimits *m_limits;
    };

    SearchOptions()
    {
        m_options["depth"] = Option(Option::setDepth, &m_limits);
        m_options["nodes"] = Option(Option::setNodes, &m_limits);
        m_options["infinite"] = Option(Option::setInfinite, &m_limits);
        m_options["ponder"] = Option(Option::setPonder, &m_limits);
        m_options["movetime"] = Option(Option::setMovetime, &m_limits);
        m_options["wtime"] = Option(Option::setWtime, &m_limits);
        m_options["btime"] = Option(Option::setBtime, &m_limits);
        m_options["winc"] = Option(Option::setWinc, &m_limits);
        m_options["binc"] = Option(Option::setBinc, &m_limits);
    }

    /**
     * @brief Set the value of an option
     */
    void setOption(const std::string& name, Option::value_t value)
    {
        auto it = m_options.find(name);
        if (it != m_options.end())
            it->second(value);
    }

    /**
     * @brief Get the `Option` object for a given name
     */
    Option& operator[](const std::string& name)
    {
        return m_options[name];
    }

    /**
     * @brief Check if an option is a boolean
     */
    bool is_boolean(const std::string& name) const
    {
        return name == "infinite" || name == "ponder";
    }

    /**
     * @brief Get the `SearchLimits` object
     */
    SearchLimits& limits() { return m_limits; }

private:
    SearchLimits m_limits;
    std::map<std::string, Option> m_options;
};

} // namespace chess