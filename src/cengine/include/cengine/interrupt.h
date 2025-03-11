#pragma once

#include <atomic>

#include "search_options.h"

namespace chess
{


// Class to handle interrupts (`stop` signal, time limit, etc.)
class Interrupt
{
    std::atomic_bool m_ignore;
    std::atomic_bool m_state;
    std::atomic_bool m_stop;
    std::atomic<uint64_t> m_nodes;
    Limits m_limits;
    chess::TimeMan m_time;


    // Resolve the state flag, based on ignore, stop and condition flags
    constexpr bool M_get_state(bool ignore, bool stop, bool state, bool cond)
    {
        return (!ignore && (state || stop || cond));
    }

public:

    Interrupt() : 
        m_ignore(false), m_state(false),
        m_stop(false), m_nodes(0),
        m_limits(), m_time() {}

    Interrupt(const Limits& limits) :
        m_ignore(false), m_state(false),
        m_stop(false), m_nodes(0), 
        m_limits(limits), m_time(limits.time) {}

    Interrupt(Interrupt&& other) 
    {
        *this = std::move(other);
    }

    Interrupt& operator=(Interrupt&& other)
    {
        m_ignore = other.m_ignore.load();
        m_stop   = other.m_stop.load();
        m_limits = other.m_limits;
        m_nodes  = other.m_nodes.load();
        m_time   = other.m_time;
        return *this;
    }

    // Set the stop signal
    void stop() { m_stop = true; }

    // Sets the ignore flag to true, will cause the search to run 
    // as if with the infinite parameter, 
    // but it cannot be stopped with `stop` signal
    // To stop this, call `restore_state`
    void set_ignore() { 
        m_ignore = true; 
        m_state  = false;
    }

    // Restore the state after ignoring
    void restore_state(int depth)
    {
        m_ignore = false;
        m_state  = M_get_state(
            false, m_stop.load(), m_state.load(),
            (m_time.get() || m_nodes.load() > m_limits.nodes 
            || depth > m_limits.depth)
        );
    }

    // Check if the stop signal is set
    bool get() const
    {
        return m_state.load();
    }

    // Update depth
    void depth(int depth)
    {
        m_state = M_get_state(
            m_ignore.load(), m_stop.load(), m_state.load(),
            depth > m_limits.depth
        );
    }

    // Increment nodes by 1, and check if the search should stop (based on time)
    void update()
    {
        add_nodes(1);
        m_time.check();
        m_state = M_get_state(
            m_ignore.load(), m_stop.load(), m_state.load(),
            m_time.get()
        );
    }

    // Update nodes
    void add_nodes(uint64_t nodes)
    {
        m_nodes += nodes;
        m_state = M_get_state(
            m_ignore.load(), m_stop.load(), m_state.load(),
            m_nodes.load() > m_limits.nodes
        );
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

    chess::TimeMan& time_man()
    {
        return m_time;
    }

    // Get `ignore` flag
    bool is_ignoring() {return m_ignore.load(); }
};

}