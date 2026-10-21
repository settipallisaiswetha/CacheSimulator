
#include <iostream>
#include <string>
#include "CacheHierarchy.h"

int passed = 0;
int failed = 0;

void check(
    const std::string& testName,
    bool condition
) {
    if (condition) {
        std::cout << "[PASS] " << testName << "\n";
        passed++;
    } else {
        std::cout << "[FAIL] " << testName << "\n";
        failed++;
    }
}

// ======================================
// TEST 1: WRITE_BACK WRITE MISS
// ======================================

void testWriteBackMiss() {
    std::cout << "\n=== TEST 1: WRITE_BACK MISS ===\n";

    Memory memory;
    memory.write(15, 500);

    CacheHierarchy hierarchy(
        1, 2,
        1, 1,
        ReplacementPolicy::LRU,
        WritePolicy::WRITE_BACK,
        memory
    );

    hierarchy.write(960, 900, 64);

    check(
        "RAM unchanged after write-back",
        memory.read(15) == 500
    );

    check(
        "L1 recorded one miss",
        hierarchy.getL1Stats().getMisses() == 1
    );

    check(
        "L2 recorded one miss",
        hierarchy.getL2Stats().getMisses() == 1
    );

    hierarchy.read(960, 64);

    check(
        "Read hits in L1",
        hierarchy.getL1Stats().getHits() == 1
    );
}

// ======================================
// TEST 2: DIRTY L1 INVALIDATION
// ======================================

void testDirtyInvalidation() {
    std::cout << "\n=== TEST 2: DIRTY INVALIDATION ===\n";

    Memory memory;
    memory.write(15, 500);
    memory.write(17, 700);

    CacheHierarchy hierarchy(
        1, 2,
        1, 1,
        ReplacementPolicy::LRU,
        WritePolicy::WRITE_BACK,
        memory
    );

    hierarchy.write(960, 900, 64);
    hierarchy.read(1088, 64);

    check(
        "Dirty data preserved in RAM",
        memory.read(15) == 900
    );

    check(
        "L1 dirty write-back recorded",
        hierarchy.getL1Stats().getWriteBacks() == 1
    );

    check(
        "L2 memory write-back recorded",
        hierarchy.getL2Stats().getWriteBacks() == 1
    );

    auto missesBefore =
        hierarchy.getL1Stats().getMisses();

    hierarchy.read(960, 64);

    check(
        "Invalidated block misses in L1",
        hierarchy.getL1Stats().getMisses()
            == missesBefore + 1
    );
}

// ======================================
// TEST 3: WRITE_THROUGH
// ======================================

void testWriteThrough() {
    std::cout << "\n=== TEST 3: WRITE_THROUGH ===\n";

    Memory memory;
    memory.write(15, 500);

    CacheHierarchy hierarchy(
        1, 2,
        1, 1,
        ReplacementPolicy::LRU,
        WritePolicy::WRITE_THROUGH,
        memory
    );

    hierarchy.write(960, 900, 64);

    check(
        "RAM updated immediately",
        memory.read(15) == 900
    );

    check(
        "No L1 write-back required",
        hierarchy.getL1Stats().getWriteBacks() == 0
    );

    check(
        "No L2 write-back required",
        hierarchy.getL2Stats().getWriteBacks() == 0
    );
}

// ======================================
// TEST 4: L1 TO L2 WRITE-BACK
// ======================================

void testL1Eviction() {
    std::cout << "\n=== TEST 4: L1 EVICTION ===\n";

    Memory memory;
    memory.write(15, 500);
    memory.write(16, 600);

    CacheHierarchy hierarchy(
        1, 1,
        2, 2,
        ReplacementPolicy::LRU,
        WritePolicy::WRITE_BACK,
        memory
    );

    hierarchy.write(960, 900, 64);
    hierarchy.read(1024, 64);

    check(
        "L1 dirty eviction recorded",
        hierarchy.getL1Stats().getWriteBacks() == 1
    );

    check(
        "RAM unchanged after L1 eviction",
        memory.read(15) == 500
    );
}

// ======================================
// TEST 5: FIFO AND RANDOM SMOKE TESTS
// ======================================

void testReplacementPolicies() {
    std::cout << "\n=== TEST 5: REPLACEMENT POLICIES ===\n";

    for (ReplacementPolicy policy : {
        ReplacementPolicy::FIFO,
        ReplacementPolicy::RANDOM
    }) {
        Memory memory;

        CacheHierarchy hierarchy(
            1, 1,
            1, 1,
            policy,
            WritePolicy::WRITE_BACK,
            memory
        );

        hierarchy.write(64, 100, 64);
        hierarchy.read(128, 64);

        check(
            policy == ReplacementPolicy::FIFO
                ? "FIFO preserves dirty data"
                : "RANDOM preserves dirty data",
            memory.read(1) == 100
        );
    }
}

// ======================================
// MAIN
// ======================================

int main() {
    testWriteBackMiss();
    testDirtyInvalidation();
    testWriteThrough();
    testL1Eviction();
    testReplacementPolicies();

    std::cout << "\n============================\n";
    std::cout << "TEST SUMMARY\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "============================\n";

    return failed == 0 ? 0 : 1;
}
