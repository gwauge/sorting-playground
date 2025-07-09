// compile using: g++ playground.cpp -o3 -o build/playground && ./build/playground
#include <chrono>
#include <vector>
#include <cstdint>
#include <array>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <thread>
#include <mutex>
// #include <execution>
#include <pdqsort.h>
#include <cstring>

#include "common.hpp"
#include "rowid.hpp"
#include "algorithms/radix.hpp"
#include "utils/timer.hpp"

// Wrapper for std::sort to match signature
void std_sort_wrapper(std::vector<ByteKey> &keys)
{
    std::sort(keys.begin(), keys.end());
}

// void std_sort_par_wrapper(std::vector<ByteKey> &keys)
// {
//     std::sort(std::execution::par, keys.begin(), keys.end());
// }

std::string print_key(const ByteKey &key)
{
    std::string result;
    for (uint8_t byte : key)
    {
        result += static_cast<char>(byte);
    }

    return result;
}

void pdqsort_wrapper(std::vector<ByteKey> &keys, std::vector<RowID> &row_ids)
{
    const size_t KEY_SIZE = 16;
    // pdqsort(keys.begin(), keys.end());
    pdqsort(row_ids.begin(), row_ids.end(),
            [&](const RowID &a, const RowID &b)
            {
                const auto &key_a = keys[CHUNK_SIZE * a.chunk_id + a.chunk_offset];
                const auto &key_b = keys[CHUNK_SIZE * b.chunk_id + b.chunk_offset];

                //   std::cout << "Comparing keys: \n";
                //   std::cout << "\t A (" << a.chunk_id << ", " << a.chunk_offset << "): " << print_key(key_a) << "\n";
                //   std::cout << "\t B (" << b.chunk_id << ", " << b.chunk_offset << "): " << print_key(key_b) << "\n";

                return memcmp(key_a.data(), key_b.data(), KEY_SIZE) < 0;
                // int compare = memcmp(key_a.data(), key_b.data(), KEY_SIZE);
                // if (compare != 0)
                //     return compare < 0;
            });
}

void print_first_n(const std::vector<ByteKey> &keys, size_t n)
{
    std::cout << "First " << n << " keys:\n";
    for (size_t i = 0; i < n && i < keys.size(); ++i)
    {
        std::cout << "Key " << i + 1 << ": " << print_key(keys[i]) << std::endl;
    }
}

void is_sorted(const std::vector<ByteKey> &keys, const std::vector<RowID> &row_ids)
{
    bool sorted = std::is_sorted(row_ids.begin(), row_ids.end(),
                                 [&](const RowID &a, const RowID &b)
                                 {
                                     const auto &key_a = keys[CHUNK_SIZE * a.chunk_id + a.chunk_offset];
                                     const auto &key_b = keys[CHUNK_SIZE * b.chunk_id + b.chunk_offset];
                                     return memcmp(key_a.data(), key_b.data(), key_a.size()) < 0;
                                 });
    std::cout << "RowIDs are " << (sorted ? "" : "NOT ") << "sorted" << std::endl;
}

int main()
{
    const size_t NUM_KEYS = getenv("NUM_KEYS", size_t(1e7));
    const size_t KEY_SIZE = getenv("KEY_SIZE", size_t(16));
    const size_t N_RUNS = getenv("N_RUNS", size_t(7)); // Number of times to benchmark each sort

    auto timer = Timer();

    timer.lap(); // Reset timer

    std::vector<RowID> row_ids;

    generate_row_ids(row_ids, NUM_KEYS);
    std::cout << "Generated " << row_ids.size() << " RowIDs in " << timer.lap_formatted() << std::endl;

    // for (const auto &row_id : row_ids)
    // {
    //     std::cout << "RowID: chunk_id=" << row_id.chunk_id << ", chunk_offset=" << row_id.chunk_offset << std::endl;
    // }

    std::cout << "Generating " << NUM_KEYS << " keys of size " << KEY_SIZE << " bytes...\n";
    std::vector<ByteKey> keys;
    generate_keys(keys, NUM_KEYS, KEY_SIZE);
    std::cout << "Key generation: " << timer.lap_formatted() << std::endl;

    auto sorted_keys = keys; // Copy for sorting
    // pdqsort_wrapper(sorted_keys, row_ids);
    // parallel_radix_wrapper(sorted_keys);
    hybrid_radix_sort_rowids_msb(row_ids, sorted_keys, CHUNK_SIZE);
    std::cout << "Sorted " << sorted_keys.size() << " keys in " << timer.lap_formatted() << std::endl;
    is_sorted(sorted_keys, row_ids);

    // Benchmark sorts
    // benchmark_sort(keys, radix_sort, N_RUNS, "Radix sort");
    // benchmark_sort(keys, std_sort_wrapper, N_RUNS, "std::sort");
    // benchmark_sort(keys, std_sort_par_wrapper, N_RUNS, "std::sort (parallel)");
    // benchmark_sort(keys, pdqsort_wrapper, N_RUNS, "pdqsort");
    // benchmark_sort(keys, parallel_radix_wrapper, N_RUNS, "radix (parallel)");
}
