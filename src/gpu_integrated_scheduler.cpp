
#include "GPUWarpMemoryAccess.h"
#include "GPULatencyAdapter.h"
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

struct Warp {
    int nextInstruction = 0;
    int readyCycle = 0;
};

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

Result simulate(int numWarps) {
    if (numWarps <= 0) {
        throw std::invalid_argument(
            "Number of warps must be positive"
        );
    }

    constexpr int instructionsPerWarp = 8;
    constexpr int blockSize = 64;

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

    // =========================================
    // ROUND-ROBIN WARP SCHEDULING
    // =========================================

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

            int index =
                (nextWarp + offset) % numWarps;

            Warp& warp = warps[index];

            if (warp.nextInstruction >=
                instructionsPerWarp) {
                continue;
            }

            if (warp.readyCycle > currentCycle) {
                continue;
            }

            // =====================================
            // GENERATE WARP MEMORY ADDRESSES
            // =====================================

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

            // =====================================
            // CACHE-LINE-AWARE MEMORY ACCESS
            // =====================================

            gpu::WarpMemoryResult access =
                gpu::analyzeWarpMemoryAccess(
                    addresses,
                    adapter,
                    blockSize
                );

            result.memorySectors +=
                access.sectors;

            result.cacheLines +=
                access.cacheLines;

            result.l1Hits += access.l1Hits;
            result.l2Hits += access.l2Hits;
            result.dramAccesses +=
                access.dramAccesses;

            // =====================================
            // MEMORY COMPLETION TIME
            // =====================================

            warp.readyCycle =
                currentCycle + access.latency;

            finalCompletionCycle = std::max(
                finalCompletionCycle,
                warp.readyCycle
            );

            // =====================================
            // ISSUE INSTRUCTION
            // =====================================

            ++warp.nextInstruction;
            ++result.issued;

            nextWarp =
                (index + 1) % numWarps;

            issued = true;
            break;
        }

        if (!issued) {
            ++result.idleCycles;
        }

        ++currentCycle;
    }

    // =========================================
    // PERFORMANCE METRICS
    // =========================================

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
        << "DRAM latency: 400 cycles\n\n";

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

    for (int count : {1, 2, 4, 8, 16, 32}) {
        Result r = simulate(count);

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
    }

    return 0;
}
