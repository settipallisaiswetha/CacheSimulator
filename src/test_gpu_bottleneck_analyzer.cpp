
#include "GPUBottleneckAnalyzer.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

bool approximatelyEqual(
    double actual,
    double expected,
    double tolerance = 0.01
) {
    return std::fabs(actual - expected) < tolerance;
}

int main() {

    // =========================================
    // TEST 1: VERIFIED 32-WARP SIMULATION
    // =========================================

    gpu::BottleneckStats stats;

    stats.issueCycles = 2832;
    stats.completionCycles = 3231;
    stats.issuedInstructions = 256;
    stats.idleCycles = 2576;

    stats.l1Hits = 0;
    stats.l2Hits = 0;
    stats.dramAccesses = 512;
    stats.cacheLines = 512;

    gpu::BottleneckReport report =
        gpu::analyzeBottlenecks(stats);

    assert(approximatelyEqual(
        report.issueUtilization,
        9.04
    ));

    assert(approximatelyEqual(
        report.totalUtilization,
        7.92
    ));

    assert(approximatelyEqual(
        report.schedulerIdlePercent,
        90.96
    ));

    assert(approximatelyEqual(
        report.completionTailPercent,
        12.35
    ));

    assert(approximatelyEqual(
        report.l1HitPercent,
        0.0
    ));

    assert(approximatelyEqual(
        report.l2HitPercent,
        0.0
    ));

    assert(approximatelyEqual(
        report.dramPercent,
        100.0
    ));

    assert(
        std::string(report.classification) ==
        "DRAM-heavy workload with scheduler idle time"
    );

    std::cout
        << "Test 1 passed: 32-warp bottleneck metrics\n";

    // =========================================
    // TEST 2: CACHE HIT PERCENTAGES
    // =========================================

    gpu::BottleneckStats cacheStats;

    cacheStats.issueCycles = 100;
    cacheStats.completionCycles = 120;
    cacheStats.issuedInstructions = 40;
    cacheStats.idleCycles = 60;

    cacheStats.l1Hits = 50;
    cacheStats.l2Hits = 30;
    cacheStats.dramAccesses = 20;
    cacheStats.cacheLines = 100;

    gpu::BottleneckReport cacheReport =
        gpu::analyzeBottlenecks(cacheStats);

    assert(approximatelyEqual(
        cacheReport.l1HitPercent,
        50.0
    ));

    assert(approximatelyEqual(
        cacheReport.l2HitPercent,
        30.0
    ));

    assert(approximatelyEqual(
        cacheReport.dramPercent,
        20.0
    ));

    std::cout
        << "Test 2 passed: Cache hit percentages\n";

    // =========================================
    // TEST 3: INVALID STATISTICS
    // =========================================

    gpu::BottleneckStats invalidStats;

    invalidStats.issueCycles = 100;
    invalidStats.completionCycles = 90;

    bool exceptionCaught = false;

    try {
        gpu::analyzeBottlenecks(invalidStats);
    }
    catch (const std::invalid_argument&) {
        exceptionCaught = true;
    }

    assert(exceptionCaught);

    std::cout
        << "Test 3 passed: Invalid statistics rejected\n";

    // =========================================
    // TEST 4: ZERO MEMORY ACCESSES
    // =========================================

    gpu::BottleneckStats emptyStats;

    gpu::BottleneckReport emptyReport =
        gpu::analyzeBottlenecks(emptyStats);

    assert(approximatelyEqual(
        emptyReport.issueUtilization,
        0.0
    ));

    assert(approximatelyEqual(
        emptyReport.dramPercent,
        0.0
    ));

    assert(
        std::string(emptyReport.classification) ==
        "No memory accesses"
    );

    std::cout
        << "Test 4 passed: Zero-access handling\n";

    std::cout
        << "\nAll GPU bottleneck analyzer tests passed!\n";

    return 0;
}
