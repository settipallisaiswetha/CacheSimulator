
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>

#include "CacheHierarchy.h"
#include "TraceReader.h"

struct CacheConfiguration {
    std::string name;
    int l1Sets;
    int l1Ways;
    int l2Sets;
    int l2Ways;
};

struct PerformanceResult {
    std::string name;
    unsigned long long accesses;
    unsigned long long l1Hits;
    unsigned long long l1Misses;
    unsigned long long l2Hits;
    unsigned long long l2Misses;
    double l1HitRate;
    double l2HitRate;
    double amat;
};

int main(int argc, char* argv[]) {
    try {
        const std::string traceFile =
            argc > 1 ? argv[1] : "traces/sample_trace.txt";

        const std::string outputFile =
            argc > 2 ? argv[2] : "comparison_results.csv";

        const auto operations =
            TraceReader::readTrace(traceFile);

        if (operations.empty()) {
            throw std::runtime_error("Trace file is empty");
        }

        const std::vector<CacheConfiguration> configurations = {
            {"Small",  2, 2,  4, 2},
            {"Medium", 4, 2,  8, 4},
            {"Large",  8, 4, 16, 4}
        };

        const int blockSize = 64;

        // Illustrative timing assumptions
        constexpr double l1Latency = 1.0;
        constexpr double l2Latency = 10.0;
        constexpr double memoryLatency = 100.0;

        std::vector<PerformanceResult> results;

        for (const auto& config : configurations) {
            // Fresh memory and cache for every configuration
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

            std::cout << "\nRunning configuration: "
                      << config.name << "\n";

            for (const auto& op : operations) {
                if (op.type == 'R') {
                    hierarchy.read(op.address, blockSize);
                } else {
                    hierarchy.write(
                        op.address,
                        op.data,
                        blockSize
                    );
                }
            }

            const auto& l1 = hierarchy.getL1Stats();
            const auto& l2 = hierarchy.getL2Stats();

            const double l1MissRate = l1.getMissRate();
            const double l2MissRate = l2.getMissRate();

            const double amat =
                l1Latency +
                l1MissRate * (
                    l2Latency +
                    l2MissRate * memoryLatency
                );

            results.push_back({
                config.name,
                l1.getAccesses(),
                l1.getHits(),
                l1.getMisses(),
                l2.getHits(),
                l2.getMisses(),
                l1.getHitRate() * 100.0,
                l2.getHitRate() * 100.0,
                amat
            });
        }

        // Print comparison table
        std::cout << "\n========== CACHE COMPARISON ==========\n";

        std::cout << std::fixed << std::setprecision(2);

        std::cout << std::left
                  << std::setw(12) << "Config"
                  << std::setw(12) << "L1 Hit %"
                  << std::setw(12) << "L2 Hit %"
                  << std::setw(14) << "AMAT(cycles)"
                  << "\n";

        for (const auto& result : results) {
            std::cout << std::left
                      << std::setw(12) << result.name
                      << std::setw(12) << result.l1HitRate
                      << std::setw(12) << result.l2HitRate
                      << std::setw(14) << result.amat
                      << "\n";
        }

        // Export CSV
        std::ofstream csv(outputFile);

        if (!csv) {
            throw std::runtime_error(
                "Cannot create CSV file: " + outputFile
            );
        }

        csv << "Configuration,Accesses,L1Hits,L1Misses,"
            << "L2Hits,L2Misses,L1HitRatePercent,"
            << "L2HitRatePercent,AMATCycles\n";

        csv << std::fixed << std::setprecision(2);

        for (const auto& result : results) {
            csv << result.name << ","
                << result.accesses << ","
                << result.l1Hits << ","
                << result.l1Misses << ","
                << result.l2Hits << ","
                << result.l2Misses << ","
                << result.l1HitRate << ","
                << result.l2HitRate << ","
                << result.amat << "\n";
        }

        csv.close();

        std::cout << "\nCSV saved to: "
                  << outputFile << "\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "ERROR: "
                  << error.what() << "\n";
        return 1;
    }
}
