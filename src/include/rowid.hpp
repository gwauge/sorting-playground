#pragma once

#include <cstdint>

class RowID
{
public:
    // Default constructor
    RowID() : chunk_id(0), chunk_offset(0) {}

    // Parameterized constructor
    RowID(uint32_t chunk_id, uint16_t chunk_offset)
        : chunk_id(chunk_id), chunk_offset(chunk_offset) {}

    // Copy constructor
    RowID(const RowID &other) = default;

    // Move constructor
    RowID(RowID &&other) noexcept = default;

    // Copy assignment
    RowID &operator=(const RowID &other) = default;

    // Move assignment
    RowID &operator=(RowID &&other) noexcept = default;

    uint32_t chunk_id;
    uint16_t chunk_offset;
};
