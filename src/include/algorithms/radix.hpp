#pragma once

#include <cstdint>
#include <cstddef>
#include <chrono>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>
#include <pdqsort.h>

#include "common.hpp"
#include "thread_pool.hpp"

constexpr size_t RADIX = 256; // byte = 0–255

// LSD Radix Sort for equal-length keys
void radix_sort(std::vector<ByteKey> &keys);

void radix_sort_parallel_msb(std::vector<ByteKey> &keys, size_t sort_byte_index = 0);

inline void parallel_radix_wrapper(std::vector<ByteKey> &keys)
{
    radix_sort_parallel_msb(keys, 0); // Sort by the 1st byte (MSB)
}

/**
 * Hybrid MSB-radix + std::sort for RowIDs.
 *
 * @param rowids        vector of RowID to sort in-place
 * @param keys          flat array of keys (index = chunk_id * CHUNKSIZE + chunk_offset)
 */
void hybrid_radix_sort_rowids_msb(
    const std::vector<ByteKey> &keys,
    std::vector<RowID> &rowids);
