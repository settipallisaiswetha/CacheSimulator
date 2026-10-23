
#pragma once

#include <cstdint>
#include <iosfwd>

namespace gpu {

struct BottleneckStats {
    std::uint64_t issueCycles = 0;
    std::uint64_t completionCycles = 0;
    std::uint64_t issuedInstructions = 0;
    std::uint64_t idleCycles = 0;

    std::uint64_t l1Hits = 0;
    std::uint64_t l2Hits = 0;
    std::uint64_t dramAccesses = 0;
    std::uint64_t cacheLines = 0;
};

struct BottleneckReport {
    double issueUtilization = 0.0;
    double totalUtilization = 0.0;

    double schedulerIdlePercent = 0.0;
    double completionTailPercent = 0.0;

    double l1HitPercent = 0.0;
    double l2HitPercent = 0.0;
    double dramPercent = 0.0;

    const char* classification = "";
};

BottleneckReport analyzeBottlenecks(
    const BottleneckStats& stats
);

void printBottleneckReport(
    std::ostream& out,
    int warps,
    const BottleneckStats& stats
);

}
