
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>

#include "CacheHierarchy.h"
#include "TraceReader.h"

struct PolicyConfig {
    std::string name;
    ReplacementPolicy policy;
};

int main(int argc, char* argv[]) {
    try {
        const std::string traceFile =
            argc > 1 ? argv[1] : "traces/repeated.txt";

        const std::string outputFile =
            argc > 2 ? argv[2] : "policy_results.csv";

        auto operations = TraceReader::readTrace(traceFile);

        if (operations.empty()) {
            throw std::runtime_error("Trace is empty");
        }

        const std::vector<PolicyConfig> policies = {
            {"FIFO", ReplacementPolicy::FIFO},
            {"LRU", ReplacementPolicy::LRU},
            {"RANDOM", ReplacementPolicy::RANDOM}
        };

        std::ofstream csv(outputFile);

        if (!csv) {
            throw std::runtime_error("Cannot create CSV file");
        }

        csv << "Policy,Accesses,L1Hits,L1Misses,"
            << "L2Hits,L2Misses,L1HitRatePercent,"
            << "L2HitRatePercent,AMATCycles\n";

        csv << std::fixed << std::setprecision(2);

        std::cout << "\n===== REPLACEMENT POLICY COMPARISON =====\n";

        for (const auto& config : policies) {
            Memory memory;

            // Use a small cache to expose replacement behavior.
            CacheHierarchy hierarchy(
                1, 2,   // L1: 1 set, 2 ways
                1, 4,   // L2: 1 set, 4 ways
                config.policy,
                WritePolicy::WRITE_BACK,
                memory
            );

            const int blockSize = 64;

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

            constexpr double l1Latency = 1.0;
            constexpr double l2Latency = 10.0;
            constexpr double ramLatency = 100.0;

            double amat =
                l1Latency +
                l1.getMissRate() * (
                    l2Latency +
                    l2.getMissRate() * ramLatency
                );

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
                      << amat << " cycles\n";
        }

        std::cout << "\nResults saved to: "
                  << outputFile << "\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << "\n";
        return 1;
    }
}
