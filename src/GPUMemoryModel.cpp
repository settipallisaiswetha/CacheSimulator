
#include "GPUMemoryModel.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <limits>

namespace gpu {

CoalescingResult analyzeCoalescing(
    const std::vector<std::uint64_t>& addresses,
    int accessSizeBytes
) {
    if (addresses.size() > WARP_SIZE) {
        throw std::invalid_argument(
            "Maximum 32 addresses per warp"
        );
    }

    if (accessSizeBytes <= 0) {
        throw std::invalid_argument(
            "Access size must be positive"
        );
    }

    std::set<std::uint64_t> sectors;

    for (std::uint64_t address : addresses) {
        const auto size =
            static_cast<std::uint64_t>(accessSizeBytes);

        if (address >
            std::numeric_limits<std::uint64_t>::max()
                - (size - 1)) {
            throw std::overflow_error(
                "Memory address range overflow"
            );
        }

        std::uint64_t first =
            address / SECTOR_SIZE;

        std::uint64_t last =
            (address + size - 1) / SECTOR_SIZE;

        for (std::uint64_t sector = first;
             sector <= last;
             ++sector) {
            sectors.insert(sector);
        }
    }

    int requestedBytes =
        static_cast<int>(addresses.size()) *
        accessSizeBytes;

    int transferredBytes =
        static_cast<int>(sectors.size()) *
        SECTOR_SIZE;

    double efficiency = transferredBytes == 0
        ? 0.0
        : 100.0 * requestedBytes / transferredBytes;

    return {
        static_cast<int>(sectors.size()),
        requestedBytes,
        transferredBytes,
        efficiency
    };
}

BankConflictResult analyzeBankConflicts(
    const std::vector<std::uint64_t>& addresses
) {
    if (addresses.size() > WARP_SIZE) {
        throw std::invalid_argument(
            "Maximum 32 addresses per warp"
        );
    }

    // Bank index -> distinct 4-byte word addresses.
    std::map<int, std::set<std::uint64_t>> bankWords;

    for (std::uint64_t address : addresses) {
        std::uint64_t word =
            address / BANK_WIDTH;

        int bank =
            static_cast<int>(word % NUM_BANKS);

        bankWords[bank].insert(word);
    }

    int maxConflict = 0;
    int conflictingBanks = 0;

    for (const auto& [bank, words] : bankWords) {
        int count = static_cast<int>(words.size());

        maxConflict = std::max(maxConflict, count);

        if (count > 1) {
            ++conflictingBanks;
        }
    }

    return {
        maxConflict,
        conflictingBanks,
        static_cast<int>(bankWords.size())
    };
}

} // namespace gpu
