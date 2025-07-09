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

// Alias for clarity
using ByteKey = std::vector<uint8_t>;

// LSD Radix Sort for equal-length keys
void radix_sort(std::vector<ByteKey> &keys)
{
    if (keys.empty())
        return;

    const size_t key_size = keys[0].size();
    const size_t RADIX = 256; // byte = 0–255

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

void radix_sort_parallel_msb(std::vector<ByteKey> &keys, size_t sort_byte_index = 0)
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
    std::vector<std::thread> threads;

    for (size_t b = 0; b < RADIX; ++b)
    {
        if (!buckets[b].empty())
        {
            threads.emplace_back([&bucket = buckets[b]]
                                 {
                                     std::sort(bucket.begin(), bucket.end()); // lexicographical for ByteKey
                                 });
        }
    }

    for (auto &t : threads)
    {
        if (t.joinable())
            t.join();
    }

    // Step 3: Reassemble sorted buckets into final array
    keys.clear();
    for (size_t b = 0; b < RADIX; ++b)
    {
        keys.insert(keys.end(),
                    std::make_move_iterator(buckets[b].begin()),
                    std::make_move_iterator(buckets[b].end()));
    }
}

// Utility: Compute median from a vector of durations
long long median(std::vector<long long> &times)
{
    std::sort(times.begin(), times.end());
    size_t n = times.size();
    if (n % 2 == 0)
        return (times[n / 2 - 1] + times[n / 2]) / 2;
    else
        return times[n / 2];
}

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
    const size_t NUM_KEYS = 10000000;
    const size_t KEY_SIZE = 32;
    const size_t N_RUNS = 7; // Number of times to benchmark each sort

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
    radix_sort_parallel_msb(sorted_keys, 0); // Sort by the 1st byte (MSB)
    // std::sort(sorted_keys.begin(), sorted_keys.end()); // Using std::sort for demonstration
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
