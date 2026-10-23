
#include "GPUMemoryQueue.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace gpu {

GPUMemoryQueue::GPUMemoryQueue(int maxOutstanding)
    : capacity(maxOutstanding) {

    if (capacity <= 0) {
        throw std::invalid_argument(
            "Memory queue capacity must be positive"
        );
    }
}

void GPUMemoryQueue::advanceTo(int currentCycle) {

    if (currentCycle < 0) {
        throw std::invalid_argument(
            "Cycle cannot be negative"
        );
    }

    while (!outstanding.empty() &&
           outstanding.top().completionCycle <= currentCycle) {

        outstanding.pop();
        ++stats.completedRequests;
    }
}

bool GPUMemoryQueue::submit(
    std::uint64_t address,
    int currentCycle,
    int latency
) {
    if (currentCycle < 0 || latency <= 0) {
        throw std::invalid_argument(
            "Invalid cycle or memory latency"
        );
    }

    if (currentCycle >
        std::numeric_limits<int>::max() - latency) {
        throw std::overflow_error(
            "Memory completion cycle overflow"
        );
    }

    // Retire completed requests before checking capacity.
    advanceTo(currentCycle);

    if (!hasCapacity()) {
        ++stats.rejectedRequests;
        return false;
    }

    MemoryRequest request;
    request.address = address;
    request.issueCycle = currentCycle;
    request.completionCycle = currentCycle + latency;

    outstanding.push(request);

    ++stats.submittedRequests;

    stats.maximumOutstanding = std::max(
        stats.maximumOutstanding,
        outstandingCount()
    );

    return true;
}

bool GPUMemoryQueue::hasCapacity() const {
    return outstandingCount() < capacity;
}

bool GPUMemoryQueue::empty() const {
    return outstanding.empty();
}

int GPUMemoryQueue::outstandingCount() const {
    return static_cast<int>(outstanding.size());
}

const MemoryQueueStats& GPUMemoryQueue::getStats() const {
    return stats;
}

}
