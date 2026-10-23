
#include "GPUMemoryQueue.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// =========================================
// GPU SIMULATION CONFIGURATION
// =========================================

constexpr int NUM_WARPS = 32;
constexpr int REQUESTS_PER_WARP = 8;
constexpr int MEMORY_LATENCY = 400;

// =========================================
// WARP STATE
// =========================================

struct Warp {
    int nextRequest = 0;
    int readyCycle = 0;
};

// =========================================
// SIMULATION RESULT
// =========================================

struct Result {
    int queueCapacity = 0;
    int issueCycles = 0;
    int completionCycles = 0;
    int issuedRequests = 0;
    int queueStallCycles = 0;
    int idleCycles = 0;
    int peakOutstanding = 0;

    double issueUtilization = 0.0;
};

// =========================================
// GPU MLP SIMULATION
// =========================================

Result simulate(int queueCapacity) {

    if (queueCapacity <= 0) {
        throw std::invalid_argument(
            "Queue capacity must be positive"
        );
    }

    gpu::GPUMemoryQueue memoryQueue(queueCapacity);

    std::vector<Warp> warps(NUM_WARPS);

    Result result;
    result.queueCapacity = queueCapacity;

    int currentCycle = 0;
    int nextWarp = 0;
    int finalCompletionCycle = 0;

    const int totalRequests =
        NUM_WARPS * REQUESTS_PER_WARP;

    // Continue until all requests have been issued.
    while (result.issuedRequests < totalRequests) {

        // Complete requests whose latency has elapsed.
        memoryQueue.advanceTo(currentCycle);

        bool issued = false;
        bool readyWarpExists = false;

        // Round-robin warp scheduling.
        for (int offset = 0; offset < NUM_WARPS; ++offset) {

            int index =
                (nextWarp + offset) % NUM_WARPS;

            Warp& warp = warps[index];

            // This warp has completed its program.
            if (warp.nextRequest >= REQUESTS_PER_WARP) {
                continue;
            }

            // This warp is waiting for a previous request.
            if (warp.readyCycle > currentCycle) {
                continue;
            }

            readyWarpExists = true;

            // Cannot issue when the memory queue is full.
            if (!memoryQueue.hasCapacity()) {
                break;
            }

            // Give each warp its own address region.
            std::uint64_t address =
                static_cast<std::uint64_t>(index)
                    * 4096ULL
                + static_cast<std::uint64_t>(
                    warp.nextRequest
                ) * 128ULL;

            bool accepted = memoryQueue.submit(
                address,
                currentCycle,
                MEMORY_LATENCY
            );

            if (!accepted) {
                throw std::runtime_error(
                    "Unexpected memory queue rejection"
                );
            }

            // Each warp has dependent memory requests.
            // It cannot issue its next request until
            // the previous one completes.
            warp.readyCycle =
                currentCycle + MEMORY_LATENCY;

            finalCompletionCycle = std::max(
                finalCompletionCycle,
                warp.readyCycle
            );

            ++warp.nextRequest;
            ++result.issuedRequests;

            // Next scheduling search begins after this warp.
            nextWarp = (index + 1) % NUM_WARPS;

            issued = true;
            break;
        }

        if (!issued) {
            ++result.idleCycles;

            // Count queue-capacity stalls separately
            // from dependency-related idle cycles.
            if (readyWarpExists &&
                !memoryQueue.hasCapacity()) {
                ++result.queueStallCycles;
            }
        }

        ++currentCycle;
    }

    result.issueCycles = currentCycle;

    // Account for the last in-flight request.
    result.completionCycles = std::max(
        currentCycle,
        finalCompletionCycle
    );

    // Retire all remaining memory requests.
    memoryQueue.advanceTo(result.completionCycles);

    const auto& stats = memoryQueue.getStats();

    if (stats.submittedRequests != totalRequests ||
        stats.completedRequests != totalRequests ||
        !memoryQueue.empty()) {

        throw std::runtime_error(
            "Memory requests were not fully completed"
        );
    }

    result.peakOutstanding =
        stats.maximumOutstanding;

    result.issueUtilization =
        100.0 * result.issuedRequests /
        result.issueCycles;

    return result;
}

// =========================================
// NUMERICAL REGRESSION VERIFICATION
// =========================================

void verifyMLPResults() {

    const int capacities[] = {
        2, 4, 8, 16, 32
    };

    // Verified simulation completion cycles.
    const int expectedCompletion[] = {
        51201,
        25603,
        12807,
        6415,
        3231
    };

    // Verified queue stall cycles.
    const int expectedQueueStalls[] = {
        50546,
        24948,
        12152,
        5760,
        0
    };

    const int expectedRequests =
        NUM_WARPS * REQUESTS_PER_WARP;

    for (int i = 0; i < 5; ++i) {

        Result result = simulate(capacities[i]);

        if (result.completionCycles !=
                expectedCompletion[i] ||

            result.queueStallCycles !=
                expectedQueueStalls[i] ||

            result.issuedRequests !=
                expectedRequests ||

            result.peakOutstanding !=
                capacities[i]) {

            throw std::runtime_error(
                "GPU MLP regression mismatch "
                "at queue capacity " +
                std::to_string(capacities[i])
            );
        }
    }

    std::cout
        << "\nAll 5 GPU MLP numerical "
        << "regression cases passed!\n";
}

} // namespace

// =========================================
// MAIN FUNCTION
// =========================================

int main() {

    try {

        std::cout
            << "\n========================================\n"
            << " GPU MEMORY-LEVEL PARALLELISM COMPARISON\n"
            << "========================================\n"
            << "Warps: " << NUM_WARPS << '\n'
            << "Memory requests per warp: "
            << REQUESTS_PER_WARP << '\n'
            << "Memory latency: "
            << MEMORY_LATENCY << " cycles\n"
            << "Issue width: 1 request/cycle\n"
            << "Requests per warp: dependent\n\n";

        std::cout
            << std::left
            << std::setw(12) << "Capacity"
            << std::setw(12) << "Issue"
            << std::setw(12) << "Complete"
            << std::setw(12) << "Issued"
            << std::setw(14) << "QueueStalls"
            << std::setw(12) << "Idle"
            << std::setw(12) << "PeakMLP"
            << "IssueUtil\n";

        std::cout
            << std::fixed
            << std::setprecision(2);

        // =====================================
        // PERFORMANCE COMPARISON
        // =====================================

        for (int capacity : {2, 4, 8, 16, 32}) {

            Result result = simulate(capacity);

            std::cout
                << std::left
                << std::setw(12)
                << result.queueCapacity

                << std::setw(12)
                << result.issueCycles

                << std::setw(12)
                << result.completionCycles

                << std::setw(12)
                << result.issuedRequests

                << std::setw(14)
                << result.queueStallCycles

                << std::setw(12)
                << result.idleCycles

                << std::setw(12)
                << result.peakOutstanding

                << result.issueUtilization
                << "%\n";
        }

        // =====================================
        // AUTOMATED REGRESSION VERIFICATION
        // =====================================

        verifyMLPResults();

        std::cout
            << "\nAll GPU MLP configurations completed!\n";

        return 0;
    }
    catch (const std::exception& error) {

        std::cerr
            << "\nGPU MLP simulation failed: "
            << error.what() << '\n';

        return 1;
    }
}
