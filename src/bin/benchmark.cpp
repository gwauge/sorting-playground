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

// Benchmarking function for any sort
// sort_fn: void(std::vector<ByteKey>&)
// Returns median time in ms
void benchmark_sort(const std::vector<ByteKey> &original_keys, void (*sort_fn)(std::vector<ByteKey> &), size_t N, const std::string &label)
{
    std::vector<long long> times;
    std::vector<ByteKey> keys;
    for (size_t i = 0; i < N; ++i)
    {
        keys = original_keys;
        auto start = std::chrono::high_resolution_clock::now();
        sort_fn(keys);
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(ms);

        // check if keys are sorted on the last run
        if (i == N - 1)
        {
            bool is_sorted = std::is_sorted(keys.begin(), keys.end());
            std::cout << label << " is " << (is_sorted ? "" : "NOT ") << "sorted\n";
        }
    }
    long long med = median(times);
    std::cout << label << " median: " << med << " ms (" << N << " runs)\n";
}

// Wrapper for std::sort to match signature
void std_sort_wrapper(std::vector<ByteKey> &keys)
{
    std::sort(keys.begin(), keys.end());
}

void parallel_radix_wrapper(std::vector<ByteKey> &keys)
{
    radix_sort_parallel_msb(keys, 0); // Sort by the 1st byte (MSB)
}

int main()
{
    const size_t NUM_KEYS = getenv("NUM_KEYS", size_t(1e7));
    const size_t KEY_SIZE = getenv("KEY_SIZE", size_t(16));
    const size_t N_RUNS = getenv("N_RUNS", size_t(7)); // Number of times to benchmark each sort

    std::cout << "Generating " << NUM_KEYS << " keys of size " << KEY_SIZE << " bytes...\n";

    std::vector<ByteKey> keys;
    keys.reserve(NUM_KEYS);

    // Timing: Key generation
    auto start_gen = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < NUM_KEYS; ++i)
    {
        ByteKey key(KEY_SIZE);
        for (size_t j = 0; j < KEY_SIZE; ++j)
        {
            key[j] = 'a' + (rand() % 26); // Random char from 'a' to 'z'
        }
        keys.emplace_back(key);
    }
    auto end_gen = std::chrono::high_resolution_clock::now();
    auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_gen - start_gen).count();
    std::cout << "Key generation: " << gen_ms << " ms" << std::endl;

    // Benchmark sorts
    // benchmark_sort(keys, radix_sort, N_RUNS, "Radix sort");
    // benchmark_sort(keys, std_sort_wrapper, N_RUNS, "std::sort");
    // benchmark_sort(keys, parallel_radix_wrapper, N_RUNS, "radix (parallel)");

    // Print first 10 sorted keys using radix_sort for demonstration
    std::vector<ByteKey> sorted_keys = keys;
    auto start = std::chrono::high_resolution_clock::now();
    // radix_sort_parallel_msb(sorted_keys, 0); // Sort by the 1st byte (MSB)
    // std::sort(sorted_keys.begin(), sorted_keys.end()); // Using std::sort for demonstration
    radix_sort(sorted_keys); // Using the original radix sort for demonstration
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Radix sort (parallel) took: " << ms << " ms" << std::endl;

    std::cout << "\nFirst 10 sorted keys (radix):\n";
    for (size_t idx = 0; idx < 10 && idx < NUM_KEYS; ++idx)
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
