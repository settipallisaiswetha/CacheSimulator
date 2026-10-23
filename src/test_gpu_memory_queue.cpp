
#include "GPUMemoryQueue.h"

#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

int main() {

    // =====================================
    // TEST 1: QUEUE CAPACITY
    // =====================================

    {
        gpu::GPUMemoryQueue queue(2);

        require(queue.submit(0, 0, 100),
                "First request should succeed");

        require(queue.submit(64, 0, 100),
                "Second request should succeed");

        require(!queue.submit(128, 0, 100),
                "Third request should be rejected");

        require(queue.outstandingCount() == 2,
                "Outstanding count should be 2");

        std::cout
            << "Test 1 passed: Queue capacity\n";
    }

    // =====================================
    // TEST 2: REQUEST COMPLETION
    // =====================================

    {
        gpu::GPUMemoryQueue queue(2);

        queue.submit(0, 0, 20);
        queue.submit(64, 0, 100);

        queue.advanceTo(19);

        require(queue.outstandingCount() == 2,
                "Requests completed too early");

        queue.advanceTo(20);

        require(queue.outstandingCount() == 1,
                "First request should complete");

        queue.advanceTo(100);

        require(queue.empty(),
                "All requests should complete");

        require(
            queue.getStats().completedRequests == 2,
            "Expected two completed requests"
        );

        std::cout
            << "Test 2 passed: Request completion\n";
    }

    // =====================================
    // TEST 3: CAPACITY REUSE
    // =====================================

    {
        gpu::GPUMemoryQueue queue(1);

        require(queue.submit(0, 0, 10),
                "Initial request should succeed");

        require(!queue.submit(64, 5, 10),
                "Queue should still be full");

        require(queue.submit(64, 10, 10),
                "Capacity should be reusable");

        require(
            queue.getStats().maximumOutstanding == 1,
            "Maximum outstanding should be 1"
        );

        std::cout
            << "Test 3 passed: Capacity reuse\n";
    }

    // =====================================
    // TEST 4: INVALID CAPACITY
    // =====================================

    {
        bool caught = false;

        try {
            gpu::GPUMemoryQueue queue(0);
        }
        catch (const std::invalid_argument&) {
            caught = true;
        }

        require(caught,
                "Invalid capacity must be rejected");

        std::cout
            << "Test 4 passed: Invalid capacity\n";
    }

    // =====================================
    // TEST 5: STATISTICS
    // =====================================

    {
        gpu::GPUMemoryQueue queue(2);

        queue.submit(0, 0, 10);
        queue.submit(64, 0, 20);
        queue.submit(128, 0, 30);

        queue.advanceTo(20);

        const auto& stats = queue.getStats();

        require(stats.submittedRequests == 2,
                "Expected 2 submitted requests");

        require(stats.completedRequests == 2,
                "Expected 2 completed requests");

        require(stats.rejectedRequests == 1,
                "Expected 1 rejected request");

        require(stats.maximumOutstanding == 2,
                "Expected maximum outstanding of 2");

        std::cout
            << "Test 5 passed: Queue statistics\n";
    }

    std::cout
        << "\nAll GPU memory queue tests passed!\n";

    return 0;
}
