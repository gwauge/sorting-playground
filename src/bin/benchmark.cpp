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

void pdqsort_wrapper(std::vector<ByteKey> &keys)
{
    pdqsort(keys.begin(), keys.end());
}

void print_first_n(const std::vector<ByteKey> &keys, size_t n)
{
    std::cout << "First " << n << " keys:\n";
    for (size_t i = 0; i < n && i < keys.size(); ++i)
    {
        std::cout << "Key " << i + 1 << ": ";
        for (uint8_t byte : keys[i])
        {
            std::cout << static_cast<char>(byte);
        }
        std::cout << std::endl;
    }
}

void is_sorted(const std::vector<ByteKey> &keys)
{
    bool sorted = std::is_sorted(keys.begin(), keys.end());
    std::cout << "Keys are " << (sorted ? "" : "NOT ") << "sorted" << std::endl;
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

    std::cout << "Generating " << NUM_KEYS << " keys of size " << KEY_SIZE << " bytes...\n";
    std::vector<ByteKey> keys;
    generate_keys(keys, NUM_KEYS, KEY_SIZE);
    std::cout << "Key generation: " << timer.lap_formatted() << std::endl;

    auto sorted_keys = keys; // Copy for sorting
    // pdqsort_wrapper(sorted_keys);
    parallel_radix_wrapper(sorted_keys);
    std::cout << "Sorted " << sorted_keys.size() << " keys in " << timer.lap_formatted() << std::endl;

    // Benchmark sorts
    // benchmark_sort(keys, radix_sort, N_RUNS, "Radix sort");
    // benchmark_sort(keys, std_sort_wrapper, N_RUNS, "std::sort");
    // benchmark_sort(keys, std_sort_par_wrapper, N_RUNS, "std::sort (parallel)");
    // benchmark_sort(keys, pdqsort_wrapper, N_RUNS, "pdqsort");
    // benchmark_sort(keys, parallel_radix_wrapper, N_RUNS, "radix (parallel)");
}
