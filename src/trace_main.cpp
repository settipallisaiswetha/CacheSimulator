
#include <iostream>
#include <iomanip>
#include <exception>

#include "CacheHierarchy.h"
#include "TraceReader.h"

int main(int argc, char* argv[]) {
    try {
        const std::string filename =
            argc > 1
                ? argv[1]
                : "traces/sample_trace.txt";

        auto operations =
            TraceReader::readTrace(filename);

        Memory memory;

        CacheHierarchy hierarchy(
            4, 2,  // L1: 4 sets, 2 ways
            8, 4,  // L2: 8 sets, 4 ways
            ReplacementPolicy::LRU,
            WritePolicy::WRITE_BACK,
            memory
        );

        const int blockSize = 64;

        for (const auto& op : operations) {
            if (op.type == 'R') {
                hierarchy.read(
                    op.address,
                    blockSize
                );
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

        std::cout << "\n========== PERFORMANCE REPORT ==========\n";

        std::cout << "Trace operations: "
                  << operations.size() << "\n";

        std::cout << "L1 accesses: "
                  << l1.getAccesses() << "\n";

        std::cout << "L1 hits: "
                  << l1.getHits() << "\n";

        std::cout << "L1 misses: "
                  << l1.getMisses() << "\n";

        std::cout << "L2 accesses: "
                  << l2.getAccesses() << "\n";

        std::cout << "L2 hits: "
                  << l2.getHits() << "\n";

        std::cout << "L2 misses: "
                  << l2.getMisses() << "\n";

        std::cout << std::fixed
                  << std::setprecision(2);

        std::cout << "L1 hit rate: "
                  << l1.getHitRate() * 100.0
                  << "%\n";

        std::cout << "L2 hit rate: "
                  << l2.getHitRate() * 100.0
                  << "%\n";

        // Illustrative latency assumptions:
        // L1 hit: 1 cycle
        // L2 access: 10 cycles
        // RAM access: 100 cycles

        constexpr double l1Latency = 1.0;
        constexpr double l2Latency = 10.0;
        constexpr double memoryLatency = 100.0;

        const double l1MissRate =
            l1.getMissRate();

        const double l2MissRate =
            l2.getMissRate();

        // Simplified demand-access AMAT model.
        // Does not include dirty eviction costs.
        const double amat =
            l1Latency +
            l1MissRate * (
                l2Latency +
                l2MissRate * memoryLatency
            );

        std::cout << "Estimated AMAT: "
                  << amat << " cycles\n";

        std::cout << "========================================\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "ERROR: "
                  << error.what() << "\n";
        return 1;
    }
}
