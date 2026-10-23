
#include "GPUWarpMemoryAccess.h"
#include "GPULatencyAdapter.h"
#include "GPUMemoryModel.h"
#include "GPUBottleneckAnalyzer.h"

#include "CacheHierarchy.h"
#include "Memory.h"
#include "ReplacementPolicy.h"
#include "WritePolicy.h"

#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

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

Result simulate(int numWarps) {

    if (numWarps <= 0) {
        throw std::invalid_argument(
            "Number of warps must be positive"
        );
    }

    constexpr int instructionsPerWarp = 8;
    constexpr int blockSize = 64;

    // =====================================
    // MEMORY AND CACHE HIERARCHY
    // =====================================

    Memory memory;

    CacheHierarchy hierarchy(
        8, 2,
        32, 4,
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
    // ROUND-ROBIN WARP SCHEDULING
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

        // Search for a ready warp
        for (int offset = 0;
             offset < numWarps;
             ++offset) {

            int index =
                (nextWarp + offset) % numWarps;

            Warp& warp = warps[index];

            // Skip completed warps
            if (warp.nextInstruction >=
                instructionsPerWarp) {
                continue;
            }

            // Skip warps waiting for memory
            if (warp.readyCycle > currentCycle) {
                continue;
            }

            // =================================
            // GENERATE WARP MEMORY ADDRESSES
            // =================================

            std::uint64_t baseAddress =
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

            // =================================
            // CACHE-LINE-AWARE MEMORY ACCESS
            // =================================

            gpu::WarpMemoryResult access =
                gpu::analyzeWarpMemoryAccess(
                    addresses,
                    adapter,
                    blockSize
                );

            // =================================
            // MEMORY STATISTICS
            // =================================

            result.memorySectors +=
                access.sectors;

            result.cacheLines +=
                access.cacheLines;

            result.l1Hits +=
                access.l1Hits;

            result.l2Hits +=
                access.l2Hits;

            result.dramAccesses +=
                access.dramAccesses;

            // =================================
            // MEMORY LATENCY
            // =================================

            warp.readyCycle =
                currentCycle + access.latency;

            finalCompletionCycle = std::max(
                finalCompletionCycle,
                warp.readyCycle
            );

            // =================================
            // ISSUE INSTRUCTION
            // =================================

            ++warp.nextInstruction;
            ++result.issued;

            nextWarp =
                (index + 1) % numWarps;

            issued = true;

            break;
        }

        // No warp ready this cycle
        if (!issued) {
            ++result.idleCycles;
        }

        ++currentCycle;
    }

    // =====================================
    // PERFORMANCE METRICS
    // =====================================

    result.issueCycles = currentCycle;

    result.completionCycles = std::max(
        currentCycle,
        finalCompletionCycle
    );

    result.issueUtilization =
        100.0 *
        static_cast<double>(result.issued) /
        static_cast<double>(result.issueCycles);

    result.totalUtilization =
        100.0 *
        static_cast<double>(result.issued) /
        static_cast<double>(result.completionCycles);

    return result;
}

// =========================================
// MAIN FUNCTION
// =========================================

int main() {

    std::cout
        << "\n========================================\n"
        << " INTEGRATED GPU PERFORMANCE SIMULATOR\n"
        << "========================================\n"
        << "Scheduling: Round Robin\n"
        << "Issue width: 1 instruction/cycle\n"
        << "Warp size: 32 threads\n"
        << "Cache block size: 64 bytes\n"
        << "L1 latency: 20 cycles\n"
        << "L2 latency: 100 cycles\n"
        << "DRAM latency: 400 cycles\n"
        << "Cache-line-aware memory access: Enabled\n\n";

    // =====================================
    // RESULTS TABLE HEADER
    // =====================================

    std::cout << std::left
              << std::setw(8) << "Warps"
              << std::setw(12) << "Issue"
              << std::setw(12) << "Complete"
              << std::setw(10) << "Issued"
              << std::setw(10) << "Idle"
              << std::setw(10) << "L1"
              << std::setw(10) << "L2"
              << std::setw(10) << "DRAM"
              << std::setw(10) << "Sectors"
              << std::setw(10) << "Lines"
              << std::setw(12) << "IssueUtil"
              << "TotalUtil\n";

    std::cout << std::fixed
              << std::setprecision(2);

    // =====================================
    // RUN DIFFERENT WARP CONFIGURATIONS
    // =====================================

    for (int count : {1, 2, 4, 8, 16, 32}) {

        Result r = simulate(count);

        // Print performance summary
        std::cout << std::left
                  << std::setw(8) << count
                  << std::setw(12) << r.issueCycles
                  << std::setw(12) << r.completionCycles
                  << std::setw(10) << r.issued
                  << std::setw(10) << r.idleCycles
                  << std::setw(10) << r.l1Hits
                  << std::setw(10) << r.l2Hits
                  << std::setw(10) << r.dramAccesses
                  << std::setw(10) << r.memorySectors
                  << std::setw(10) << r.cacheLines
                  << std::setw(12)
                  << r.issueUtilization
                  << r.totalUtilization
                  << "%\n";

        // =================================
        // GPU BOTTLENECK ANALYSIS
        // =================================

        if (count == 32) {

            gpu::BottleneckStats stats;

            stats.issueCycles =
                r.issueCycles;

            stats.completionCycles =
                r.completionCycles;

            stats.issuedInstructions =
                r.issued;

            stats.idleCycles =
                r.idleCycles;

            stats.l1Hits =
                r.l1Hits;

            stats.l2Hits =
                r.l2Hits;

            stats.dramAccesses =
                r.dramAccesses;

            stats.cacheLines =
                r.cacheLines;

            gpu::printBottleneckReport(
                std::cout,
                count,
                stats
            );
        }
    }

    return 0;
}
