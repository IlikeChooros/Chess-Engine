#pragma once

#include <atomic>

#include "search_options.h"

namespace chess
{


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
    void update()
    {
        add_nodes(1);
        m_time.check();
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

}