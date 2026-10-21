
#include "GPUWarpMemoryAccess.h"

#include <set>
#include <stdexcept>
#include <algorithm>

namespace gpu {

WarpMemoryResult analyzeWarpMemoryAccess(
    const std::vector<std::uint64_t>& addresses,
    GPULatencyAdapter& adapter,
    int blockSize
) {
    if (blockSize <= 0) {
        throw std::invalid_argument(
            "Block size must be positive"
        );
    }

    WarpMemoryResult result;

    auto coalescing = analyzeCoalescing(addresses, 4);
    result.sectors = coalescing.sectors;

    std::set<std::uint64_t> uniqueLines;

    for (std::uint64_t address : addresses) {
        uniqueLines.insert(
            address / static_cast<std::uint64_t>(blockSize)
        );
    }

    result.cacheLines =
        static_cast<int>(uniqueLines.size());

    for (std::uint64_t line : uniqueLines) {
        std::uint64_t address =
            line * static_cast<std::uint64_t>(blockSize);

        auto access = adapter.access(address);

        if (access.level == MemoryLevel::L1) {
            ++result.l1Hits;
        }
        else if (access.level == MemoryLevel::L2) {
            ++result.l2Hits;
        }
        else {
            ++result.dramAccesses;
        }

        // Simplified assumption:
        // All cache-line requests overlap perfectly.
        // The slowest access determines warp latency.
        result.latency = std::max(
            result.latency,
            access.latency
        );
    }

    return result;
}

}
