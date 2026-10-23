
#pragma once

#include <cstdint>
#include <queue>
#include <vector>

namespace gpu {

struct MemoryRequest {
    std::uint64_t address = 0;
    int issueCycle = 0;
    int completionCycle = 0;
};

struct MemoryQueueStats {
    int submittedRequests = 0;
    int completedRequests = 0;
    int rejectedRequests = 0;
    int maximumOutstanding = 0;
};

class GPUMemoryQueue {
public:
    explicit GPUMemoryQueue(int maxOutstanding);

    bool submit(
        std::uint64_t address,
        int currentCycle,
        int latency
    );

    void advanceTo(int currentCycle);

    bool hasCapacity() const;
    bool empty() const;

    int outstandingCount() const;

    const MemoryQueueStats& getStats() const;

private:
    struct CompareCompletion {
        bool operator()(
            const MemoryRequest& a,
            const MemoryRequest& b
        ) const {
            return a.completionCycle > b.completionCycle;
        }
    };

    int capacity;

    std::priority_queue<
        MemoryRequest,
        std::vector<MemoryRequest>,
        CompareCompletion
    > outstanding;

    MemoryQueueStats stats;
};

}
