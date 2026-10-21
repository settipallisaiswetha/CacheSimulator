
#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <cstdint>

constexpr int WARP_SIZE = 32;
constexpr int NUM_BANKS = 32;
constexpr int ELEMENT_BYTES = 4;
constexpr int SECTOR_BYTES = 32;
constexpr int TILE_SIZE = 32;

struct Result {
    std::string name;
    int globalSectors;
    int sharedConflict;
    double globalEfficiency;
};

// Count unique 32-byte global-memory sectors.
int countSectors(
    std::uint64_t base,
    int rowStride,
    bool columnAccess
) {
    std::set<std::uint64_t> sectors;

    for (int lane = 0; lane < WARP_SIZE; ++lane) {
        std::uint64_t elementIndex =
            columnAccess
                ? static_cast<std::uint64_t>(lane) * rowStride
                : static_cast<std::uint64_t>(lane);

        std::uint64_t address =
            base + elementIndex * ELEMENT_BYTES;

        const std::uint64_t first =
            address / SECTOR_BYTES;

        const std::uint64_t last =
            (address + ELEMENT_BYTES - 1) / SECTOR_BYTES;

        for (auto sector = first; sector <= last; ++sector) {
            sectors.insert(sector);
        }
    }

    return static_cast<int>(sectors.size());
}

// Count maximum number of distinct shared-memory
// words accessed in any one bank.
int sharedBankConflict(
    int rowStride,
    bool columnAccess
) {
    std::map<int, std::set<int>> bankWords;

    for (int lane = 0; lane < WARP_SIZE; ++lane) {
        int wordIndex =
            columnAccess ? lane * rowStride : lane;

        int bank = wordIndex % NUM_BANKS;

        bankWords[bank].insert(wordIndex);
    }

    int maximum = 0;

    for (const auto& [bank, words] : bankWords) {
        maximum = std::max(
            maximum,
            static_cast<int>(words.size())
        );
    }

    return maximum;
}

Result simulate(
    const std::string& name,
    bool globalColumnAccess,
    bool sharedColumnAccess,
    int sharedRowStride
) {
    int sectors = countSectors(
        0,
        TILE_SIZE,
        globalColumnAccess
    );

    int conflict = sharedColumnAccess
        ? sharedBankConflict(sharedRowStride, true)
        : 1;

    double efficiency =
        100.0 * (WARP_SIZE * ELEMENT_BYTES) /
        (sectors * SECTOR_BYTES);

    return {name, sectors, conflict, efficiency};
}

int main() {
    const Result naive = simulate(
        "Naive transpose",
        true,
        false,
        TILE_SIZE
    );

    const Result tiled = simulate(
        "Tiled (no padding)",
        false,
        true,
        TILE_SIZE
    );

    const Result padded = simulate(
        "Tiled (padding)",
        false,
        true,
        TILE_SIZE + 1
    );

    std::cout
        << "\nGPU MATRIX TRANSPOSE MEMORY MODEL\n"
        << "Warp size: 32\n"
        << "Tile size: 32 x 32\n"
        << "Element size: 4 bytes\n"
        << "Global sector size: 32 bytes\n"
        << "Shared memory banks: 32\n\n";

    std::cout << std::left
              << std::setw(22) << "Method"
              << std::setw(12) << "Sectors"
              << std::setw(17) << "Bank Conflict"
              << "Global Efficiency\n";

    std::cout << std::fixed << std::setprecision(2);

    for (const Result& result : {naive, tiled, padded}) {
        std::cout << std::left
                  << std::setw(22) << result.name
                  << std::setw(12) << result.globalSectors
                  << std::setw(17) << result.sharedConflict
                  << result.globalEfficiency << "%\n";
    }

    return 0;
}
