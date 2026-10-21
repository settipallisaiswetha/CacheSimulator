
#include "GPULatencyAdapter.h"

#include <stdexcept>

GPULatencyAdapter::GPULatencyAdapter(
    CacheHierarchy& hierarchy,
    int blockSize
)
    : hierarchy(hierarchy),
      blockSize(blockSize)
{
    if (blockSize <= 0) {
        throw std::invalid_argument(
            "Block size must be positive"
        );
    }
}

MemoryAccessResult GPULatencyAdapter::access(
    unsigned long long address
) {
    // Record statistics before accessing memory.
    const auto l1HitsBefore =
        hierarchy.getL1Stats().getHits();

    const auto l1MissesBefore =
        hierarchy.getL1Stats().getMisses();

    const auto l2HitsBefore =
        hierarchy.getL2Stats().getHits();

    const auto l2MissesBefore =
        hierarchy.getL2Stats().getMisses();

    // Perform the actual cache hierarchy access.
    hierarchy.read(address, blockSize);

    // Determine which memory level served the request.
    if (hierarchy.getL1Stats().getHits() >
        l1HitsBefore) {
        return {MemoryLevel::L1, 20};
    }

    if (hierarchy.getL1Stats().getMisses() >
            l1MissesBefore &&
        hierarchy.getL2Stats().getHits() >
            l2HitsBefore) {
        return {MemoryLevel::L2, 100};
    }

    if (hierarchy.getL2Stats().getMisses() >
        l2MissesBefore) {
        return {MemoryLevel::DRAM, 400};
    }

    throw std::runtime_error(
        "Unable to classify memory access"
    );
}
