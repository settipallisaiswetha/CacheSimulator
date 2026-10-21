
#include <iostream>
#include <string>

#include "Cache.h"
#include "CacheStats.h"
#include "Memory.h"
#include "ReplacementPolicy.h"
#include "WritePolicy.h"

int passed = 0;
int failed = 0;

void check(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << name << "\n";
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << "\n";
        ++failed;
    }
}

// Test replacement behavior directly in a 2-way cache.
// All blocks map to set 0.
void testPolicy(
    ReplacementPolicy policy,
    const std::string& name,
    unsigned long long expectedVictim
) {
    std::cout << "\n=== " << name << " TEST ===\n";

    Memory memory;
    CacheStats stats;

    Cache cache(
        1, 2,
        policy,
        WritePolicy::WRITE_BACK,
        memory
    );

    // Block 0
    cache.fillBlockForHierarchy(
        0, 0, 0, 100, false, stats
    );

    // Block 1
    cache.fillBlockForHierarchy(
        1, 0, 1, 200, false, stats
    );

    // Access block 0 again.
    // LRU should now consider block 1 least recently used.
    // FIFO should still consider block 0 oldest.
    unsigned long long data = 0;

    bool hit = cache.getData(0, 0, data);

    check(name + ": block 0 hit", hit && data == 100);

    // Evict one block using the configured policy.
    unsigned long long victimAddress = 0;
    unsigned long long victimData = 0;
    bool victimDirty = false;

    bool evicted = cache.evictBlock(
        0,
        victimAddress,
        victimData,
        victimDirty
    );

    check(name + ": eviction succeeded", evicted);

    check(
        name + ": correct victim selected",
        evicted && victimAddress == expectedVictim
    );

    check(
        name + ": victim is invalidated",
        evicted &&
        !cache.lookup(expectedVictim, 0)
    );

    unsigned long long remainingBlock =
        expectedVictim == 0 ? 1 : 0;

    check(
        name + ": other block remains",
        cache.lookup(remainingBlock, 0)
    );
}

int main() {
    testPolicy(
        ReplacementPolicy::FIFO,
        "FIFO",
        0
    );

    testPolicy(
        ReplacementPolicy::LRU,
        "LRU",
        1
    );

    std::cout << "\n============================\n";
    std::cout << "REPLACEMENT TEST SUMMARY\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "============================\n";

    return failed == 0 ? 0 : 1;
}
