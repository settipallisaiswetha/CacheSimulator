
#pragma once

#include "CacheHierarchy.h"

enum class MemoryLevel {
    L1,
    L2,
    DRAM
};

struct MemoryAccessResult {
    MemoryLevel level;
    int latency;
};

class GPULatencyAdapter {
private:
    CacheHierarchy& hierarchy;
    int blockSize;

public:
    GPULatencyAdapter(
        CacheHierarchy& hierarchy,
        int blockSize
    );

    MemoryAccessResult access(
        unsigned long long address
    );
};
