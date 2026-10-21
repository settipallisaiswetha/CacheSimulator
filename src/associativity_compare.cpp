
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>

#include "CacheHierarchy.h"
#include "TraceReader.h"

struct Configuration {
    std::string name;
    int l1Sets;
    int l1Ways;
    int l2Sets;
    int l2Ways;
};

int main(int argc, char* argv[]) {
    try {
        const std::string traceFile =
            argc > 1 ? argv[1] : "traces/stride_8.txt";

        const std::string outputFile =
            argc > 2 ? argv[2] : "associativity_results.csv";

        const auto operations =
            TraceReader::readTrace(traceFile);

        if (operations.empty()) {
            throw std::runtime_error("Trace file is empty");
        }

        // Constant capacity:
        // L1 = 32 blocks = 2048 bytes
        // L2 = 64 blocks = 4096 bytes
        const std::vector<Configuration> configurations = {
            {"DirectMapped", 32, 1, 64, 1},
            {"2Way",         16, 2, 32, 2},
            {"4Way",          8, 4, 16, 4},
            {"FullyAssoc",    1, 32, 1, 64}
        };

        constexpr int blockSize = 64;

        constexpr double l1Latency = 1.0;
        constexpr double l2Latency = 10.0;
        constexpr double memoryLatency = 100.0;

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

        std::cout << "\n===== ASSOCIATIVITY COMPARISON =====\n";

        for (const auto& config : configurations) {
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

            for (const auto& op : operations) {
                if (op.type == 'R') {
                    hierarchy.read(op.address, blockSize);
                } else if (op.type == 'W') {
                    hierarchy.write(
                        op.address,
                        op.data,
                        blockSize
                    );
                }
            }

            const auto& l1 = hierarchy.getL1Stats();
            const auto& l2 = hierarchy.getL2Stats();

            double amat =
                l1Latency +
                l1.getMissRate() *
                (l2Latency +
                 l2.getMissRate() * memoryLatency);

            csv << config.name << ","
                << l1.getAccesses() << ","
                << l1.getHits() << ","
                << l1.getMisses() << ","
                << l2.getHits() << ","
                << l2.getMisses() << ","
                << l1.getHitRate() * 100.0 << ","
                << l2.getHitRate() * 100.0 << ","
                << amat << "\n";

            std::cout << std::fixed << std::setprecision(2)
                      << config.name
                      << " | L1 Hit: "
                      << l1.getHitRate() * 100.0
                      << "%"
                      << " | L2 Hit: "
                      << l2.getHitRate() * 100.0
                      << "%"
                      << " | AMAT: "
                      << amat
                      << " cycles\n";
        }

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
