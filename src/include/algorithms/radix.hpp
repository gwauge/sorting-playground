#include <cstdint>
#include <cstddef>

#include "common.h"

constexpr size_t RADIX = 256; // byte = 0–255

// LSD Radix Sort for equal-length keys
void radix_sort(std::vector<ByteKey> &keys);
