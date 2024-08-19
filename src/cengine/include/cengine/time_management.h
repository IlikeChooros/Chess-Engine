#pragma once

#include <chrono>


// Time management class, when created it will keep track of the time
class TimeManagement
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
    bool infinite;
public:
    TimeManagement(
        bool infinite = false,
        int movetime = 0
    )
    {
        reset(infinite, movetime);
    }

    TimeManagement(const TimeManagement& other)
    {
        *this = other;
    }

    TimeManagement& operator=(const TimeManagement& other)
    {
        start_time = other.start_time;
        end_time = other.end_time;
        infinite = other.infinite;
        return *this;
    }

    TimeManagement& operator=(TimeManagement&& other)
    {
        start_time = std::move(other.start_time);
        end_time = std::move(other.end_time);
        infinite = other.infinite;
        return *this;
    }

    void reset(bool infinite, int movetime)
    {
        this->infinite = infinite;
        this->start_time = std::chrono::high_resolution_clock::now();
        if (!infinite)
            this->end_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(movetime);
    }

    // Check if the time has ended
    bool end() 
    {
        using namespace std::chrono;
        return !infinite && high_resolution_clock::now() >= end_time;
    }

    // Get the elapsed time in milliseconds (from start to now)
    size_t elapsed() 
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(high_resolution_clock::now() - start_time).count();
    }
};