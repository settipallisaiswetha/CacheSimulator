
#pragma once

#include "GPULatencyAdapter.h"
#include "GPUMemoryModel.h"

#include <cstdint>
#include <vector>

namespace gpu {

struct WarpMemoryResult {
    int sectors = 0;
    int cacheLines = 0;

    int l1Hits = 0;
    int l2Hits = 0;
    int dramAccesses = 0;

    int latency = 0;
};

WarpMemoryResult analyzeWarpMemoryAccess(
    const std::vector<std::uint64_t>& addresses,
    GPULatencyAdapter& adapter,
    int blockSize = 64
);

}
