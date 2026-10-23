
#include "GPUWarpMemoryAccess.h"
#include "GPULatencyAdapter.h"
#include "GPUMemoryModel.h"
#include "GPUBottleneckAnalyzer.h"

#include "CacheHierarchy.h"
#include "Memory.h"
#include "ReplacementPolicy.h"
#include "WritePolicy.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

// =========================================
// CACHE CONFIGURATION
// =========================================

struct CacheConfig {
    const char* name;
    int l1Sets;
    int l1Ways;
    int l2Sets;
    int l2Ways;
};

// =========================================
// WARP STATE
// =========================================

struct Warp {
    int nextInstruction = 0;
    int readyCycle = 0;
};

// =========================================
// SIMULATION RESULTS
// =========================================

struct Result {
    int issueCycles = 0;
    int completionCycles = 0;
    int issued = 0;
    int idleCycles = 0;

    int l1Hits = 0;
    int l2Hits = 0;
    int dramAccesses = 0;

    int memorySectors = 0;
    int cacheLines = 0;

    double issueUtilization = 0.0;
    double totalUtilization = 0.0;
};

// =========================================
// GPU SIMULATION
// =========================================

Result simulate(
    const CacheConfig& config,
    int numWarps
) {
    if (numWarps <= 0) {
        throw std::invalid_argument(
            "Number of warps must be positive"
        );
    }

    constexpr int instructionsPerWarp = 8;
    constexpr int blockSize = 64;

    Memory memory;

    CacheHierarchy hierarchy(
        config.l1Sets,
        config.l1Ways,
        config.l2Sets,
        config.l2Ways,
        ReplacementPolicy::LRU,
        WritePolicy::WRITE_BACK,
        memory
    );

    GPULatencyAdapter adapter(
        hierarchy,
        blockSize
    );

    std::vector<Warp> warps(numWarps);

    Result result;

    int nextWarp = 0;
    int currentCycle = 0;
    int finalCompletionCycle = 0;

    // =====================================
    // ROUND-ROBIN WARP SCHEDULER
    // =====================================

    while (true) {
        bool unfinished = false;

        for (const auto& warp : warps) {
            if (warp.nextInstruction <
                instructionsPerWarp) {
                unfinished = true;
                break;
            }
        }

        if (!unfinished) {
            break;
        }

        bool issued = false;

        for (int offset = 0;
             offset < numWarps;
             ++offset) {

            const int index =
                (nextWarp + offset) % numWarps;

            Warp& warp = warps[index];

            if (warp.nextInstruction >=
                instructionsPerWarp) {
                continue;
            }

            if (warp.readyCycle > currentCycle) {
                continue;
            }

            // Memory access pattern:
            // Each warp accesses 128 bytes.
            // Consecutive instruction pairs
            // reuse the same memory region.

            const std::uint64_t baseAddress =
                static_cast<std::uint64_t>(index)
                    * 4096ULL
                +
                static_cast<std::uint64_t>(
                    warp.nextInstruction / 2
                ) * 128ULL;

            std::vector<std::uint64_t> addresses;
            addresses.reserve(gpu::WARP_SIZE);

            for (int lane = 0;
                 lane < gpu::WARP_SIZE;
                 ++lane) {

                addresses.push_back(
                    baseAddress
                    +
                    static_cast<std::uint64_t>(lane)
                        * 4ULL
                );
            }

            // Analyze coalescing and cache access
            const gpu::WarpMemoryResult access =
                gpu::analyzeWarpMemoryAccess(
                    addresses,
                    adapter,
                    blockSize
                );

            result.memorySectors += access.sectors;
            result.cacheLines += access.cacheLines;

            result.l1Hits += access.l1Hits;
            result.l2Hits += access.l2Hits;
            result.dramAccesses += access.dramAccesses;

            // Model instruction dependency latency
            warp.readyCycle =
                currentCycle + access.latency;

            finalCompletionCycle = std::max(
                finalCompletionCycle,
                warp.readyCycle
            );

            ++warp.nextInstruction;
            ++result.issued;

            nextWarp = (index + 1) % numWarps;

            issued = true;
            break;
        }

        if (!issued) {
            ++result.idleCycles;
        }

        ++currentCycle;
    }

    // =====================================
    // FINAL PERFORMANCE METRICS
    // =====================================

    result.issueCycles = currentCycle;

    result.completionCycles = std::max(
        currentCycle,
        finalCompletionCycle
    );

    result.issueUtilization =
        100.0 * result.issued /
        result.issueCycles;

    result.totalUtilization =
        100.0 * result.issued /
        result.completionCycles;

    return result;
}

// =========================================
// MAIN: AUTOMATED CACHE SWEEP
// =========================================

int main() {
    constexpr int blockSize = 64;

    // Four cache configurations
    const std::vector<CacheConfig> configurations = {
        {"Baseline", 8, 2, 32, 4},
        {"Config_A", 8, 4, 32, 8},
        {"Config_B", 8, 8, 32, 16},
        {"Config_C", 8, 16, 32, 16}
    };

    // Four warp counts
    const std::vector<int> warpCounts = {
        4, 8, 16, 32
    };

    // =====================================
    // CREATE CSV FILE
    // =====================================

    std::ofstream csv("gpu_cache_sweep_results.csv");

    if (!csv.is_open()) {
        std::cerr
            << "Error: Cannot create CSV file.\n";
        return 1;
    }

    csv << "Warps,Configuration,"
        << "L1Sets,L1Ways,"
        << "L2Sets,L2Ways,"
        << "L1Hits,L2Hits,"
        << "DRAMAccesses,"
        << "MemorySectors,CacheLines,"
        << "IssueCycles,CompletionCycles,"
        << "IssuedInstructions,IdleCycles,"
        << "IssueUtilization,TotalUtilization,"
        << "DRAMPercent,SchedulerIdlePercent,"
        << "Classification\n";

    // =====================================
    // PRINT HEADER
    // =====================================

    std::cout
        << "\n========================================\n"
        << " AUTOMATED GPU CACHE CONFIGURATION SWEEP\n"
        << "========================================\n"
        << "Warp counts: 4, 8, 16, 32\n"
        << "Cache configurations: 4\n"
        << "Total experiments: 16\n"
        << "Cache block size: " << blockSize
        << " bytes\n"
        << "Replacement policy: LRU\n"
        << "Write policy: WRITE_BACK\n"
        << "Issue width: 1 instruction/cycle\n\n";

    std::cout << std::left
              << std::setw(8)  << "Warps"
              << std::setw(12) << "Config"
              << std::setw(10) << "L1Hits"
              << std::setw(10) << "L2Hits"
              << std::setw(10) << "DRAM"
              << std::setw(10) << "Issue"
              << std::setw(12) << "Complete"
              << std::setw(12) << "IssueUtil"
              << "TotalUtil\n";

    std::cout << std::fixed
              << std::setprecision(2);

    csv << std::fixed
        << std::setprecision(4);

    int experimentCount = 0;

    // =====================================
    // RUN ALL 16 EXPERIMENTS
    // =====================================

    for (int warps : warpCounts) {

        for (const auto& config : configurations) {

            const Result r = simulate(
                config,
                warps
            );

            // =================================
            // BOTTLENECK ANALYSIS
            // =================================

            gpu::BottleneckStats stats;

            stats.issueCycles = r.issueCycles;
            stats.completionCycles = r.completionCycles;
            stats.issuedInstructions = r.issued;
            stats.idleCycles = r.idleCycles;

            stats.l1Hits = r.l1Hits;
            stats.l2Hits = r.l2Hits;
            stats.dramAccesses = r.dramAccesses;
            stats.cacheLines = r.cacheLines;

            const gpu::BottleneckReport report =
                gpu::analyzeBottlenecks(stats);

            // =================================
            // CONSOLE OUTPUT
            // =================================

            std::cout << std::left
                      << std::setw(8)  << warps
                      << std::setw(12) << config.name
                      << std::setw(10) << r.l1Hits
                      << std::setw(10) << r.l2Hits
                      << std::setw(10) << r.dramAccesses
                      << std::setw(10) << r.issueCycles
                      << std::setw(12) << r.completionCycles
                      << std::setw(12) << r.issueUtilization
                      << r.totalUtilization << "%\n";

            // =================================
            // CSV OUTPUT
            // =================================

            csv << warps << ','
                << config.name << ','
                << config.l1Sets << ','
                << config.l1Ways << ','
                << config.l2Sets << ','
                << config.l2Ways << ','
                << r.l1Hits << ','
                << r.l2Hits << ','
                << r.dramAccesses << ','
                << r.memorySectors << ','
                << r.cacheLines << ','
                << r.issueCycles << ','
                << r.completionCycles << ','
                << r.issued << ','
                << r.idleCycles << ','
                << r.issueUtilization << ','
                << r.totalUtilization << ','
                << report.dramPercent << ','
                << report.schedulerIdlePercent << ','
                << report.classification
                << '\n';

            ++experimentCount;
        }
    }

    csv.close();

    if (!csv) {
        std::cerr
            << "Error: Failed to write CSV file.\n";
        return 1;
    }

    // =====================================
    // COMPLETION MESSAGE
    // =====================================

    std::cout
        << "\n========================================\n"
        << " SWEEP COMPLETE\n"
        << "========================================\n"
        << "Experiments completed: "
        << experimentCount << '\n'
        << "CSV file: gpu_cache_sweep_results.csv\n";

    return 0;
}
