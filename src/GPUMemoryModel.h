
#pragma once

#include <cstdint>
#include <vector>

namespace gpu {

constexpr int WARP_SIZE = 32;
constexpr int NUM_BANKS = 32;
constexpr int BANK_WIDTH = 4;
constexpr int SECTOR_SIZE = 32;

struct CoalescingResult {
    int sectors;
    int requestedBytes;
    int transferredBytes;
    double efficiencyPercent;
};

struct BankConflictResult {
    int maxConflict;
    int conflictingBanks;
    int uniqueBanks;
};

CoalescingResult analyzeCoalescing(
    const std::vector<std::uint64_t>& addresses,
    int accessSizeBytes = 4
);

BankConflictResult analyzeBankConflicts(
    const std::vector<std::uint64_t>& addresses
);

} // namespace gpu
