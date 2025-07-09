#pragma once

#include <cstdint>

class RowID
{
public:
    RowID(uint32_t chunk_id, uint16_t chunk_offset)
        : m_chunk_id(chunk_id), m_chunk_offset(chunk_offset) {}

    uint32_t chunk_id() const { return m_chunk_id; }
    uint16_t chunk_offset() const { return m_chunk_offset; }

private:
    uint32_t m_chunk_id;
    uint16_t m_chunk_offset;
};
