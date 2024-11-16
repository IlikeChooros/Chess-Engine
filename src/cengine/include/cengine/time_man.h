#pragma once

#include <atomic>
#include <chrono>
#include <limits>

#include "types.h"

namespace chess
{

// Time limits
struct TimeLimits
{
    // Time to search in milliseconds
    uint64_t movetime = std::numeric_limits<uint64_t>::max();
    // Time for white and black [black, white]
    uint64_t time[2] = {0, 0};
    // Increment for white and black [black, white]
    uint64_t inc[2] = {0, 0};
    // Infinite search, i.e. search until stop signal
    bool infinite = false;

    // Copy operator
    constexpr TimeLimits& operator=(const TimeLimits& other)
    {
        memcpy(this, &other, sizeof(TimeLimits));
        return *this;
    }
};

/*

## Time Management

The `TimeMan` class is used to manage the time in the game while
searching for the best move.

#### Prediction

This feature is used to predict the time taken to search a given
depth. Engine may decide whether to stop the search or not, based
on the time taken to search a given depth.

*/
class TimeMan
{
public:
    typedef uint64_t Time;
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
    
    // Convert time points to milliseconds
    constexpr static uint64_t to_ms(time_point start, time_point end)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    } 

    // Get the current time
    static time_point now()
    {
        return std::chrono::high_resolution_clock::now();
    }

    /// Constructors
    TimeMan(): m_stop(false), m_limits({}), m_start_time(now()) {}
    TimeMan(const TimeLimits& limits): m_stop(false), m_limits(limits), m_start_time(now()) {}
    TimeMan(const TimeMan& other)
    {
        *this = other;
    }

    TimeMan& operator=(const TimeMan& other)
    {
        m_stop       = other.m_stop.load();
        m_limits     = other.m_limits;
        m_start_time = other.m_start_time;
        return *this;
    }

    // Start the timer
    void start()
    {
        m_start_time = std::chrono::high_resolution_clock::now();
    }

    // See if the game should stop
    bool get() { return m_stop.load(); }

    // Check if the game should stop, updates the stop flag
    void check(bool side)
    {
        if (m_limits.infinite)
            return;

        auto time = elapsed();

        if (time >= m_limits.movetime)
            m_stop = true;

        // if (time >= m_limits.time[side])
        //     m_stop = true;
    }

    // Update the time, increment the time (if any)
    void update(bool side)
    {
        // if (m_limits.infinite)
        //     return;

        // m_limits.time[side] += m_limits.inc[side];
    }

    // Get the time elapsed in milliseconds
    Time elapsed() const
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(now() - m_start_time).count();
    } 

private:
    std::atomic_bool m_stop;
    TimeLimits m_limits;
    time_point m_start_time;
};

}