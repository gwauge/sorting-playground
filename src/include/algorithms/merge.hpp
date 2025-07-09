#pragma once

#include <vector>

#include "rowid.hpp"
#include "common.hpp"

void merge_sort(
    const std::vector<ByteKey> &keys,
    std::vector<RowID> &rowids);
