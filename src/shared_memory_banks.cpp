
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>

constexpr int WARP_SIZE = 32;
constexpr int NUM_BANKS = 32;
constexpr int BANK_WIDTH = 4; // 4 bytes per bank word

struct Result {
    std::string pattern;
    int maxConflict;
    int conflictingBanks;
    int uniqueBanks;
};

// Each lane accesses one 32-bit shared-memory word.
// Different words in the same bank cause a conflict.
// Multiple lanes accessing the same word are treated
// as a broadcast, not a bank conflict.
Result simulate(
    const std::string& name,
    int strideWords,
    bool broadcast = false
) {
    // bank -> distinct word addresses
    std::map<int, std::set<int>> bankWords;

    for (int lane = 0; lane < WARP_SIZE; ++lane) {
        int wordAddress = broadcast
            ? 0
            : lane * strideWords;

        int bank = wordAddress % NUM_BANKS;

        bankWords[bank].insert(wordAddress);
    }

    int maxConflict = 0;
    int conflictingBanks = 0;

    for (const auto& [bank, words] : bankWords) {
        int distinctWords =
            static_cast<int>(words.size());

        maxConflict = std::max(
            maxConflict,
            distinctWords
        );

        if (distinctWords > 1) {
            ++conflictingBanks;
        }
    }

    return {
        name,
        maxConflict,
        conflictingBanks,
        static_cast<int>(bankWords.size())
    };
}

int main() {
    std::vector<Result> results = {
        simulate("Contiguous", 1),
        simulate("Stride 2", 2),
        simulate("Stride 4", 4),
        simulate("Stride 8", 8),
        simulate("Stride 16", 16),
        simulate("Stride 32", 32),
        simulate("Broadcast", 0, true)
    };

    std::cout << "\nGPU SHARED MEMORY BANK CONFLICT SIMULATOR\n";
    std::cout << "Warp size: 32 threads\n";
    std::cout << "Number of banks: 32\n";
    std::cout << "Bank width: "
              << BANK_WIDTH << " bytes\n\n";

    std::cout << std::left
              << std::setw(16) << "Pattern"
              << std::setw(16) << "Max Conflict"
              << std::setw(20) << "Conflicting Banks"
              << "Unique Banks\n";

    for (const auto& result : results) {
        std::cout << std::left
                  << std::setw(16) << result.pattern
                  << std::setw(16) << result.maxConflict
                  << std::setw(20) << result.conflictingBanks
                  << result.uniqueBanks
                  << "\n";
    }

    return 0;
}
