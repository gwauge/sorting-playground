#include <algorithm>
#include <vector>
#include <future>
#include <cstring>
#include "thread_pool.hpp"
#include "algorithms/merge.hpp"
#include "rowid.hpp"
#include <pdqsort.h>

struct RowIDKeyComparator
{
    const std::vector<ByteKey> &keys;
    size_t key_size;
    RowIDKeyComparator(const std::vector<ByteKey> &keys, size_t key_size)
        : keys(keys), key_size(key_size) {}
    bool operator()(const RowID &a, const RowID &b) const
    {
        const auto &key_a = keys[CHUNK_SIZE * a.chunk_id + a.chunk_offset];
        const auto &key_b = keys[CHUNK_SIZE * b.chunk_id + b.chunk_offset];
        return memcmp(key_a.data(), key_b.data(), key_size) < 0;
    }
};

void merge_sort(
    const std::vector<ByteKey> &keys,
    std::vector<RowID> &rowids)
{
    if (rowids.empty())
        return;
    const size_t key_size = keys[0].size();
    const size_t num_threads = std::max<size_t>(2, std::thread::hardware_concurrency());
    const size_t chunk_size = (rowids.size() + num_threads - 1) / num_threads;
    std::vector<std::vector<RowID>> chunks;
    chunks.reserve(num_threads);

    // Split rowids into chunks (move, not copy)
    auto it = rowids.begin();
    for (size_t i = 0; i < num_threads && it != rowids.end(); ++i)
    {
        size_t this_chunk_size = std::min(chunk_size, static_cast<size_t>(rowids.end() - it));
        std::vector<RowID> chunk;
        chunk.reserve(this_chunk_size);
        for (size_t j = 0; j < this_chunk_size && it != rowids.end(); ++j, ++it)
        {
            chunk.push_back(std::move(*it));
        }
        chunks.push_back(std::move(chunk));
    }
    rowids.clear();

    ThreadPool pool(num_threads);
    RowIDKeyComparator cmp(keys, key_size);
    std::vector<std::future<void>> sort_futures;

    // Sort each chunk in parallel
    for (auto &chunk : chunks)
    {
        sort_futures.push_back(pool.enqueue([&cmp, &chunk]
                                            { pdqsort(chunk.begin(), chunk.end(), cmp); }));
    }
    for (auto &fut : sort_futures)
        fut.get();

    // Merge chunks in parallel until <=2 remain
    while (chunks.size() > 2)
    {
        std::vector<std::vector<RowID>> next_chunks;
        std::vector<std::future<std::vector<RowID>>> merge_futures;
        for (size_t i = 0; i + 1 < chunks.size(); i += 2)
        {
            auto &left = chunks[i];
            auto &right = chunks[i + 1];
            merge_futures.push_back(pool.enqueue([&cmp, l = std::move(left), r = std::move(right)]() mutable
                                                 {
                std::vector<RowID> merged;
                merged.reserve(l.size() + r.size());
                std::merge(std::make_move_iterator(l.begin()), std::make_move_iterator(l.end()),
                           std::make_move_iterator(r.begin()), std::make_move_iterator(r.end()),
                           std::back_inserter(merged), cmp);
                return merged; }));
        }
        // If odd chunk out, just move it to next round
        if (chunks.size() % 2 == 1)
        {
            next_chunks.push_back(std::move(chunks.back()));
        }
        // Collect merged results
        for (auto &fut : merge_futures)
        {
            next_chunks.push_back(fut.get());
        }
        chunks = std::move(next_chunks);
    }

    // Final merge single-threaded, move semantics
    if (chunks.size() == 2)
    {
        std::vector<RowID> merged;
        merged.reserve(chunks[0].size() + chunks[1].size());
        std::merge(std::make_move_iterator(chunks[0].begin()), std::make_move_iterator(chunks[0].end()),
                   std::make_move_iterator(chunks[1].begin()), std::make_move_iterator(chunks[1].end()),
                   std::back_inserter(merged), cmp);
        rowids = std::move(merged);
    }
    else if (chunks.size() == 1)
    {
        rowids = std::move(chunks[0]);
    }
}
