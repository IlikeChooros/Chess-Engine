#pragma once

#include <mutex>
#include <list>
#include <map>

#include "move.h"
#include "eval.h"
#include "time_man.h"

namespace chess
{

constexpr int MIN = -(1 << 20),
             MAX = 1 << 20,
             MATE = -(1 << 19) + 1,
             MATE_THRESHOLD = -MATE;

// Score struct
struct Score
{
    // Type of score (either centipawns or mate)
    enum Type{
        cp, mate
    } type = cp;

    // Value of the score
    Value value = 0;
};

// Updates the score based on the given evaluation
inline void update_score(Score& score, int eval, int whotomove, int pv_length = 0)
{
    // Update the score
    score.type = Score::cp;
    score.value = eval * whotomove;
    
    // If that's a mate score
    if (abs(eval) >= MATE_THRESHOLD)
    {
        score.type = Score::mate;
        // Get the mate in moves, from eval
        // score.value = MATE_THRESHOLD + depth - abs(eval);
        // score.value = std::max(score.value, 1);
        // score.value *= eval > 0 ? 1 : -1;
        // score.value *= whotomove;

        score.value = (pv_length + 1) / 2;
        score.value = std::max(score.value, 1);
        score.value *= eval > 0 ? 1 : -1;
        score.value *= whotomove;
    }
}

// Search result
struct Result
{
    Move bestmove      = Move::nullMove;
    Score score        = {Score::cp, 0};
    MoveList pv        = {};
    Termination status = NONE;
    Depth depth        = 0;
    uint64_t nodes     = 0;
};

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

// Main object to handle search options
class SearchOptions
{
public:

    // Search option
    class Option
    {
    public:
        typedef int value_t;
        typedef std::function<void(Limits&, value_t value)> fn_setter_t;

        // Default setters
        static void defaultSetter(Limits&, value_t) {}
        static void setDepth(Limits& params, value_t value) { params.depth = value; }
        static void setNodes(Limits& params, value_t value) { params.nodes = value; }
        static void setPonder(Limits& params, value_t value) { params.ponder = bool(value); }

        // Time related setters
        static void setInfinite(Limits& params, value_t value) { params.time.infinite = bool(value); }
        static void setMovetime(Limits& params, value_t value) { params.time.movetime = value; }
        static void setWtime(Limits& params, value_t value) { params.time.time[1] = value; }
        static void setBtime(Limits& params, value_t value) { params.time.time[0] = value; }
        static void setWinc(Limits& params, value_t value) { params.time.inc[1] = value; }
        static void setBinc(Limits& params, value_t value) { params.time.inc[0] = value; }

        // Constructors
        Option(fn_setter_t setter = defaultSetter) : m_setter(setter), m_limits(nullptr) {}
        Option(fn_setter_t setter, Limits* param): m_setter(setter), m_limits(param) {}
        Option(const Option& other): m_setter(other.m_setter), m_limits(other.m_limits) {}

        Option& operator=(const Option& other) 
        { 
            m_limits = other.m_limits;
            m_setter = other.m_setter; 
            return *this; 
        }

        // Set the setter
        Option& operator=(fn_setter_t setter) { m_setter = setter; return *this; }

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
        fn_setter_t m_setter;
        Limits *m_limits;
    };

    SearchOptions()
    {
        m_options["depth"]    = Option(Option::setDepth, &m_limits);
        m_options["nodes"]    = Option(Option::setNodes, &m_limits);
        m_options["infinite"] = Option(Option::setInfinite, &m_limits);
        m_options["ponder"]   = Option(Option::setPonder, &m_limits);
        m_options["movetime"] = Option(Option::setMovetime, &m_limits);
        m_options["wtime"]    = Option(Option::setWtime, &m_limits);
        m_options["btime"]    = Option(Option::setBtime, &m_limits);
        m_options["winc"]     = Option(Option::setWinc, &m_limits);
        m_options["binc"]     = Option(Option::setBinc, &m_limits);
    }

    /**
     * @brief Set the value of an option
     */
    void setOption(const std::string& name, Option::value_t value)
    {
        if (name == "perft")
        {
            m_perft = value;
            return;
        }

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

    bool is_perft() const
    {
        return m_perft != 0;
    }

    Depth perft() const
    {
        return m_perft;
    }

    /**
     * @brief Get the `Limits` object
     */
    Limits& limits() { return m_limits; }
    const Limits& limits() const { return m_limits; }

\
    friend bool operator==(const SearchOptions& lhs, const Limits& rhs)
    {
        return M_eq_limits(lhs.limits(), rhs);
    }

    // Compare the search options (only the limits)
    friend bool operator==(const SearchOptions& lhs, const SearchOptions& rhs)
    {
        return M_eq_limits(lhs.limits(), rhs.limits());
    }

private:
    Depth m_perft = 0;
    Limits m_limits;
    std::map<std::string, Option> m_options;

    // Compare the limits
    static bool M_eq_limits(const Limits& lhs_limits, const Limits& rhs_limits)
    {
        // Basic cmp of depth, nodes, mate, movetime, infinite
        if (lhs_limits.depth != rhs_limits.depth ||
            lhs_limits.nodes != rhs_limits.nodes ||
            lhs_limits.mate != rhs_limits.mate ||
            lhs_limits.ponder != rhs_limits.ponder ||
            lhs_limits.time.movetime != rhs_limits.time.movetime ||
            lhs_limits.time.infinite != rhs_limits.time.infinite )
            return false;
        
        // Compare the time & increment limits
        if (lhs_limits.time.time[0] != rhs_limits.time.time[0] ||
            lhs_limits.time.time[1] != rhs_limits.time.time[1] ||
            lhs_limits.time.inc[0] != rhs_limits.time.inc[0] ||
            lhs_limits.time.inc[1] != rhs_limits.time.inc[1])
            return false;
        
        // Limits are equal
        return true;
    }
};

} // namespace chess