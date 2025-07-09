#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

#include "rowid.hpp"

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

// Benchmarking function for any sort
// sort_fn: void(std::vector<ByteKey>&)
// Returns median time in ms
inline void benchmark_sort(const std::vector<ByteKey> &original_keys, void (*sort_fn)(std::vector<ByteKey> &), size_t N, const std::string &label)
{
    std::vector<long long> times;
    std::vector<ByteKey> keys;
    for (size_t i = 0; i < N; ++i)
    {
        keys = original_keys;
        auto start = std::chrono::high_resolution_clock::now();
        sort_fn(keys);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(ms);

        // check if keys are sorted on the last run
        if (i == N - 1)
        {
            bool is_sorted = std::is_sorted(keys.begin(), keys.end());
            std::cout << label << " is " << (is_sorted ? "" : "NOT ") << "sorted\n";
        }
    }
    long long med = median(times);
    std::cout << label << " median: " << med << " ms (" << N << " runs)\n";
}

inline void generate_keys(std::vector<ByteKey> &keys, size_t num_keys, size_t key_size)
{
    keys.reserve(num_keys);
    for (size_t i = 0; i < num_keys; ++i)
    {
        ByteKey key(key_size);
        for (size_t j = 0; j < key_size; ++j)
        {
            key[j] = 'a' + (rand() % 26); // Random char from 'a' to 'z'
        }
        keys.emplace_back(std::move(key));
    }
}

inline void generate_row_ids(
    std::vector<RowID> &row_ids,
    const size_t num_keys,
    const uint16_t chunk_size = std::numeric_limits<uint16_t>::max())
{
    const uint16_t CHUNK_SIZE = std::numeric_limits<uint16_t>::max();
    const uint32_t NUM_CHUNKS = (num_keys + CHUNK_SIZE - 1) / CHUNK_SIZE; // Round up division
    std::cout << "Using " << NUM_CHUNKS << " chunks of size " << CHUNK_SIZE << std::endl;

    for (uint32_t chunk_id = 0; chunk_id < NUM_CHUNKS; ++chunk_id)
    {
        auto num_rows_in_chunk = std::min(num_keys - chunk_id * CHUNK_SIZE, static_cast<size_t>(CHUNK_SIZE));
        for (uint16_t chunk_offset = 0; chunk_offset < num_rows_in_chunk; ++chunk_offset)
        {
            row_ids.emplace_back(chunk_id, chunk_offset);
        }
    }
}
