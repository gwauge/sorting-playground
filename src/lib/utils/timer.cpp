#include "utils/timer.hpp"

#include <chrono>
#include <string>

Timer::Timer()
{
    _begin = std::chrono::steady_clock::now();
}

std::chrono::nanoseconds Timer::lap()
{
    const auto now = std::chrono::steady_clock::now();
    const auto lap_duration = std::chrono::nanoseconds{now - _begin};
    _begin = now;
    return lap_duration;
}

std::string Timer::lap_formatted()
{
    auto duration = lap();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    if (ms > 100)
    {
        return std::to_string(ms) + " ms";
    }
    else
    {
        double ms_precise = duration.count() / 1'000'000.0;
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.3f ms", ms_precise);
        return std::string(buffer);
    }
}
