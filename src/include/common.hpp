#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

// Alias for clarity
using ByteKey = std::vector<uint8_t>;

// Compute median from a vector of durations
inline long long median(std::vector<long long> &times)
{
    std::sort(times.begin(), times.end());
    size_t n = times.size();
    if (n % 2 == 0)
        return (times[n / 2 - 1] + times[n / 2]) / 2;
    else
        return times[n / 2];
}

inline size_t getenv(const char *name, size_t default_value)
{
    const char *value = std::getenv(name);
    if (value)
    {
        try
        {
            return size_t{std::stoull(value)};
        }
        catch (const std::exception &)
        {
            // If conversion fails, return default value
            return default_value;
        }
    }
    return default_value;
}
