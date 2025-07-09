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

#include "common.hpp"
#include "algorithms/radix.hpp"

// Wrapper for std::sort to match signature
void std_sort_wrapper(std::vector<ByteKey> &keys)
{
    std::sort(keys.begin(), keys.end());
}

void run_single_algorithm(std::vector<ByteKey> &sorted_keys,
                          void (*sort_algo)(std::vector<ByteKey> &))
{
    auto start = std::chrono::high_resolution_clock::now();
    sort_algo(sorted_keys); // Replace with the actual sorting algorithm
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Sort took: " << ms << " ms" << std::endl;

    // Print first 10 sorted keys using radix_sort for demonstration
    std::cout << "\nFirst 10 sorted keys (radix):\n";
    for (size_t idx = 0; idx < 10; ++idx)
    {
        const auto &key = sorted_keys[idx];
        std::cout << "Key " << idx + 1 << ": ";
        for (uint8_t byte : key)
        {
            std::cout << static_cast<char>(byte);
        }
        std::cout << std::endl;
    }

    bool is_sorted = std::is_sorted(sorted_keys.begin(), sorted_keys.end());
    std::cout << "sorted_keys is " << (is_sorted ? "" : "NOT ") << "sorted" << std::endl;
}

int main()
{
    const size_t NUM_KEYS = getenv("NUM_KEYS", size_t(1e7));
    const size_t KEY_SIZE = getenv("KEY_SIZE", size_t(16));
    const size_t N_RUNS = getenv("N_RUNS", size_t(7)); // Number of times to benchmark each sort

    std::cout << "Generating " << NUM_KEYS << " keys of size " << KEY_SIZE << " bytes...\n";
    std::vector<ByteKey> keys;

    // Timing: Key generation
    auto start_gen = std::chrono::high_resolution_clock::now();
    generate_keys(keys, NUM_KEYS, KEY_SIZE);
    auto end_gen = std::chrono::high_resolution_clock::now();
    auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_gen - start_gen).count();
    std::cout << "Key generation: " << gen_ms << " ms" << std::endl;

    // Benchmark sorts
    // benchmark_sort(keys, radix_sort, N_RUNS, "Radix sort");
    // benchmark_sort(keys, std_sort_wrapper, N_RUNS, "std::sort");
    // benchmark_sort(keys, parallel_radix_wrapper, N_RUNS, "radix (parallel)");

    run_single_algorithm(keys, parallel_radix_wrapper);
}
