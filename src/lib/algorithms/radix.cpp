#include <chrono>
#include <vector>
#include <cstdint>
#include <array>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>

#include "algorithms/radix.hpp"

void radix_sort(std::vector<ByteKey> &keys)
{
    if (keys.empty())
        return;

    const size_t key_size = keys[0].size();
    // const size_t RADIX = 256; // byte = 0–255

    // Check all keys are the same size
    for (const auto &key : keys)
    {
        if (key.size() != key_size)
        {
            throw std::invalid_argument("All keys must have the same length.");
        }
    }

    std::vector<ByteKey> temp(keys.size());

    for (int byte_index = static_cast<int>(key_size) - 1; byte_index >= 0; --byte_index)
    {
        // Counting sort buckets
        std::array<size_t, RADIX> count = {};
        std::array<size_t, RADIX> prefix_sum = {};

        // Count occurrences of each byte value at position byte_index
        for (const auto &key : keys)
        {
            uint8_t b = key[byte_index];
            count[b]++;
        }

        // Compute prefix sum to determine positions
        size_t sum = 0;
        for (size_t i = 0; i < RADIX; ++i)
        {
            prefix_sum[i] = sum;
            sum += count[i];
        }

        // Place keys in temp array based on current byte
        for (const auto &key : keys)
        {
            uint8_t b = key[byte_index];
            temp[prefix_sum[b]++] = key;
        }

        // Copy back to original array
        keys = temp;
    }
}
