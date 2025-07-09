#pragma once

#include <cstdint>
#include <cstddef>

#include "common.hpp"

constexpr size_t RADIX = 256; // byte = 0–255

// LSD Radix Sort for equal-length keys
void radix_sort(std::vector<ByteKey> &keys);

void radix_sort_parallel_msb(std::vector<ByteKey> &keys, size_t sort_byte_index);
void radix_sort_parallel_msb(std::vector<ByteKey> &keys);
