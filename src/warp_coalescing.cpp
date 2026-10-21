
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <string>
#include <cstdint>

constexpr int WARP_SIZE = 32;
constexpr std::uint64_t ELEMENT_SIZE = 4;
constexpr std::uint64_t SECTOR_SIZE = 32;

struct Result {
    std::string pattern;
    int uniqueSectors;
    int requestedBytes;
    int transferredBytes;
    double efficiency;
};

Result simulate(
    const std::string& name,
    std::uint64_t baseAddress,
    int stride
) {
    std::set<std::uint64_t> sectors;

    for (int lane = 0; lane < WARP_SIZE; ++lane) {
        std::uint64_t address =
            baseAddress +
            static_cast<std::uint64_t>(lane) *
            stride * ELEMENT_SIZE;

        // Account for every sector touched by an element.
        std::uint64_t firstSector =
            address / SECTOR_SIZE;

        std::uint64_t lastSector =
            (address + ELEMENT_SIZE - 1) / SECTOR_SIZE;

        for (std::uint64_t sector = firstSector;
             sector <= lastSector;
             ++sector) {
            sectors.insert(sector);
        }
    }

    int requestedBytes = WARP_SIZE * ELEMENT_SIZE;
    int transferredBytes =
        static_cast<int>(sectors.size() * SECTOR_SIZE);

    double efficiency =
        100.0 * requestedBytes / transferredBytes;

    return {
        name,
        static_cast<int>(sectors.size()),
        requestedBytes,
        transferredBytes,
        efficiency
    };
}

int main() {
    std::vector<Result> results = {
        simulate("Contiguous", 0, 1),
        simulate("Stride 2", 0, 2),
        simulate("Stride 4", 0, 4),
        simulate("Stride 8", 0, 8),
        simulate("Misaligned", 4, 1)
    };

    std::cout << "\nGPU WARP MEMORY COALESCING\n";
    std::cout << "Warp size: 32 threads\n";
    std::cout << "Element size: 4 bytes\n";
    std::cout << "Sector size: 32 bytes\n\n";

    std::cout << std::left
              << std::setw(15) << "Pattern"
              << std::setw(12) << "Sectors"
              << std::setw(16) << "Requested(B)"
              << std::setw(18) << "Transferred(B)"
              << "Efficiency\n";

    std::cout << std::fixed << std::setprecision(2);

    for (const auto& result : results) {
        std::cout << std::left
                  << std::setw(15) << result.pattern
                  << std::setw(12) << result.uniqueSectors
                  << std::setw(16) << result.requestedBytes
                  << std::setw(18) << result.transferredBytes
                  << result.efficiency << "%\n";
    }

    return 0;
}
