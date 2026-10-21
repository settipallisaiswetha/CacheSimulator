
#include "GPUMemoryModel.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>

// Generate memory addresses for 32 GPU threads
std::vector<std::uint64_t> makeAddresses(
    int strideWords
) {
    std::vector<std::uint64_t> addresses;

    for (int lane = 0; lane < gpu::WARP_SIZE; ++lane) {
        addresses.push_back(
            static_cast<std::uint64_t>(lane) *
            strideWords * gpu::BANK_WIDTH
        );
    }

    return addresses;
}

int main() {

    // Create CSV output file
    std::ofstream csv("gpu_memory_results.csv");

    if (!csv.is_open()) {
        std::cerr << "Error: Cannot create CSV file!\n";
        return 1;
    }

    // CSV column headings
    csv << "Pattern,Sectors,EfficiencyPercent,BankConflict\n";

    std::cout
        << "\nINTEGRATED GPU MEMORY PERFORMANCE MODEL\n\n";

    std::cout << "Warp Size: " << gpu::WARP_SIZE << "\n";
    std::cout << "Memory Sector Size: "
              << gpu::SECTOR_SIZE << " bytes\n";
    std::cout << "Shared Memory Banks: "
              << gpu::NUM_BANKS << "\n\n";

    std::cout << std::left
              << std::setw(16) << "Pattern"
              << std::setw(12) << "Sectors"
              << std::setw(16) << "Efficiency"
              << std::setw(16) << "Bank Conflict"
              << "\n";

    std::cout << std::string(60, '-') << "\n";

    // Test different memory access strides
    for (int stride : {1, 2, 4, 8, 16, 32}) {

        // Generate warp memory addresses
        auto addresses = makeAddresses(stride);

        // Analyze global memory coalescing
        auto global =
            gpu::analyzeCoalescing(addresses);

        // Analyze shared memory bank conflicts
        auto shared =
            gpu::analyzeBankConflicts(addresses);

        std::string pattern =
            "Stride " + std::to_string(stride);

        // Print results to terminal
        std::cout << std::left
                  << std::setw(16) << pattern
                  << std::setw(12) << global.sectors
                  << std::setw(16)
                  << std::fixed << std::setprecision(2)
                  << global.efficiencyPercent
                  << shared.maxConflict
                  << "\n";

        // Save results to CSV
        csv << pattern << ","
            << global.sectors << ","
            << std::fixed << std::setprecision(2)
            << global.efficiencyPercent << ","
            << shared.maxConflict
            << "\n";
    }

    csv.close();

    std::cout
        << "\nGPU performance analysis completed.\n";

    std::cout
        << "Results saved to gpu_memory_results.csv\n";

    return 0;
}
