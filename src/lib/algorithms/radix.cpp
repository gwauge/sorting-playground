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

void radix_sort_parallel_msb(std::vector<ByteKey> &keys, size_t sort_byte_index)
{
    if (keys.empty())
        return;

    const size_t key_size = keys[0].size();
    const size_t RADIX = 256;

    for (const auto &key : keys)
    {
        if (key.size() != key_size)
            throw std::invalid_argument("All keys must have the same length.");
    }

    // Step 1: Distribute keys into 256 buckets by the byte at sort_byte_index
    std::array<std::vector<ByteKey>, RADIX> buckets;
    for (auto &key : keys)
    {
        uint8_t b = key[sort_byte_index];
        buckets[b].push_back(std::move(key));
    }

    // Step 2: Sort each bucket in parallel
    ThreadPool pool; // Uses hardware concurrency by default
    std::vector<std::future<void>> futures;

    for (size_t b = 0; b < RADIX; ++b)
    {
        if (!buckets[b].empty())
        {
            futures.push_back(pool.enqueue([&bucket = buckets[b]]
                                           { std::sort(bucket.begin(), bucket.end()); }));
        }
    }

    for (auto &fut : futures)
        fut.get();

    // Step 3: Reassemble sorted buckets into final array
    keys.clear();
    for (size_t b = 0; b < RADIX; ++b)
    {
        keys.insert(keys.end(),
                    std::make_move_iterator(buckets[b].begin()),
                    std::make_move_iterator(buckets[b].end()));
    }
}

void hybrid_radix_sort_rowids_msb(
    const std::vector<ByteKey> &keys,
    std::vector<RowID> &rowids)
{
    if (rowids.empty())
        return;
    constexpr size_t RADIX = 256;
    constexpr size_t msb_index = 0; // which byte to bucket on (0 = most significant)

    // 1) Create empty buckets
    std::array<std::vector<RowID>, RADIX> buckets;

    // 2) Distribute by MSB
    for (auto &rid : rowids)
    {
        size_t idx = rid.chunk_id * CHUNK_SIZE + rid.chunk_offset;
        uint8_t b = keys[idx][msb_index];
        buckets[b].push_back(rid);
    }

    // 3) Sort each bucket in parallel
    ThreadPool pool; // Uses hardware concurrency by default
    std::vector<std::future<void>> futures;

    for (size_t b = 0; b < RADIX; ++b)
    {
        if (buckets[b].empty())
            continue;
        // spawn a thread up to hw
        futures.push_back(pool.enqueue(
            [&buckets, &keys, b]()
            {
                auto &bucket = buckets[b];
                pdqsort(bucket.begin(), bucket.end(),
                        [&](const RowID &a, const RowID &c)
                        {
                            const auto &A = keys[a.chunk_id * CHUNK_SIZE + a.chunk_offset];
                            const auto &C = keys[c.chunk_id * CHUNK_SIZE + c.chunk_offset];
                            return A < C;
                        });
            }));
    }

    // wait on any worker threads
    for (auto &fut : futures)
        fut.get();

    // 4) Gather back in bucket order
    rowids.clear();
    for (size_t b = 0; b < RADIX; ++b)
    {
        auto &bucket = buckets[b];
        rowids.insert(
            rowids.end(),
            std::make_move_iterator(bucket.begin()),
            std::make_move_iterator(bucket.end()));
    }
}
