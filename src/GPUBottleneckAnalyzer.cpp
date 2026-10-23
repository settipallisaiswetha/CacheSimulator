
#include "GPUBottleneckAnalyzer.h"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <stdexcept>

namespace gpu {

namespace {

double percent(
    std::uint64_t numerator,
    std::uint64_t denominator
) {
    if (denominator == 0) {
        return 0.0;
    }

    return 100.0 *
        static_cast<double>(numerator) /
        static_cast<double>(denominator);
}

}

BottleneckReport analyzeBottlenecks(
    const BottleneckStats& stats
) {
    if (stats.completionCycles < stats.issueCycles ||
        stats.issuedInstructions > stats.issueCycles ||
        stats.idleCycles + stats.issuedInstructions !=
            stats.issueCycles ||
        stats.l1Hits + stats.l2Hits +
            stats.dramAccesses != stats.cacheLines) {
        throw std::invalid_argument(
            "Inconsistent GPU bottleneck statistics"
        );
    }

    BottleneckReport report;

    report.issueUtilization = percent(
        stats.issuedInstructions,
        stats.issueCycles
    );

    report.totalUtilization = percent(
        stats.issuedInstructions,
        stats.completionCycles
    );

    report.schedulerIdlePercent = percent(
        stats.idleCycles,
        stats.issueCycles
    );

    report.completionTailPercent = percent(
        stats.completionCycles - stats.issueCycles,
        stats.completionCycles
    );

    report.l1HitPercent = percent(
        stats.l1Hits,
        stats.cacheLines
    );

    report.l2HitPercent = percent(
        stats.l2Hits,
        stats.cacheLines
    );

    report.dramPercent = percent(
        stats.dramAccesses,
        stats.cacheLines
    );

    // Heuristic classification, not hardware diagnosis.
    if (stats.cacheLines == 0) {
        report.classification =
            "No memory accesses";
    }
    else if (report.dramPercent >= 75.0 &&
             report.schedulerIdlePercent >= 50.0) {
        report.classification =
            "DRAM-heavy workload with scheduler idle time";
    }
    else if (report.schedulerIdlePercent >= 50.0) {
        report.classification =
            "High scheduler idle time";
    }
    else if (report.dramPercent >= 75.0) {
        report.classification =
            "DRAM-heavy workload";
    }
    else {
        report.classification =
            "Mixed cache and scheduling behavior";
    }

    return report;
}

void printBottleneckReport(
    std::ostream& out,
    int warps,
    const BottleneckStats& stats
) {
    const BottleneckReport r =
        analyzeBottlenecks(stats);

    out << "\n========================================\n"
        << " GPU PERFORMANCE BOTTLENECK ANALYSIS\n"
        << "========================================\n"
        << "Active warps: " << warps << '\n'
        << "Issue cycles: " << stats.issueCycles << '\n'
        << "Completion cycles: "
        << stats.completionCycles << '\n'
        << "Issued instructions: "
        << stats.issuedInstructions << '\n';

    out << std::fixed << std::setprecision(2)
        << "Issue utilization: "
        << r.issueUtilization << "%\n"
        << "Total utilization: "
        << r.totalUtilization << "%\n"
        << "Scheduler idle fraction: "
        << r.schedulerIdlePercent << "%\n"
        << "Completion tail fraction: "
        << r.completionTailPercent << "%\n"
        << "L1 hit fraction: "
        << r.l1HitPercent << "%\n"
        << "L2 hit fraction: "
        << r.l2HitPercent << "%\n"
        << "DRAM access fraction: "
        << r.dramPercent << "%\n"
        << "Classification: "
        << r.classification << '\n';
}

}
