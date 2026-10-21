
#include "GPULatencyAdapter.h"
#include "Memory.h"
#include "ReplacementPolicy.h"
#include "WritePolicy.h"

#include <iostream>
#include <stdexcept>

void check(
    bool condition,
    const char* message
) {
    if (!condition) {
        throw std::runtime_error(message);
    }

    std::cout << "[PASS] " << message << '\n';
}

int main() {
    try {
        Memory memory;

        CacheHierarchy hierarchy(
            1, 1,  // L1: 1 set, 1 way
            2, 1,  // L2: 2 sets, 1 way
            ReplacementPolicy::LRU,
            WritePolicy::WRITE_BACK,
            memory
        );

        GPULatencyAdapter adapter(hierarchy, 64);

        // First access: not cached.
        auto first = adapter.access(0);

        check(
            first.level == MemoryLevel::DRAM &&
            first.latency == 400,
            "Cold access uses DRAM"
        );

        // Same address: should hit in L1.
        auto second = adapter.access(0);

        check(
            second.level == MemoryLevel::L1 &&
            second.latency == 20,
            "Repeated access hits L1"
        );

        // Address 64 maps to another L2 set,
        // but the same L1 set.
        auto third = adapter.access(64);

        check(
            third.level == MemoryLevel::DRAM,
            "Second block initially misses both caches"
        );

        // Block 0 was evicted from L1 but remains in L2.
        auto fourth = adapter.access(0);

        check(
            fourth.level == MemoryLevel::L2 &&
            fourth.latency == 100,
            "Previously evicted L1 block hits L2"
        );

        std::cout
            << "\nGPU LATENCY ADAPTER TEST PASSED\n";

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr
            << "[FAIL] " << e.what() << '\n';

        return 1;
    }
}
