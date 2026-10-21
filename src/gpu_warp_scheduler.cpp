
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

struct Warp {
    int id;
    int instructionsRemaining;
    int readyCycle = 0;
};

struct SimulationResult {
    int totalCycles;
    int issuedInstructions;
    int idleCycles;
};

SimulationResult simulate(
    int numWarps,
    int instructionsPerWarp,
    int instructionLatency
) {
    std::vector<Warp> warps;

    for (int i = 0; i < numWarps; ++i) {
        warps.push_back({i, instructionsPerWarp, 0});
    }

    int cycle = 0;
    int issued = 0;
    int idle = 0;
    int nextWarp = 0;

    while (true) {
        bool completed = true;

        for (const auto& warp : warps) {
            if (warp.instructionsRemaining > 0) {
                completed = false;
                break;
            }
        }

        if (completed) {
            break;
        }

        bool scheduled = false;

        // Round-robin search for a ready warp
        for (int offset = 0; offset < numWarps; ++offset) {
            int index = (nextWarp + offset) % numWarps;
            Warp& warp = warps[index];

            if (warp.instructionsRemaining > 0 &&
                warp.readyCycle <= cycle) {

                --warp.instructionsRemaining;
                warp.readyCycle = cycle + instructionLatency;

                ++issued;
                nextWarp = (index + 1) % numWarps;
                scheduled = true;
                break;
            }
        }

        if (!scheduled) {
            ++idle;
        }

        ++cycle;
    }

    return {cycle, issued, idle};
}

int main() {
    constexpr int INSTRUCTIONS_PER_WARP = 10;
    constexpr int INSTRUCTION_LATENCY = 8;

    std::cout
        << "\nGPU WARP SCHEDULING AND LATENCY MODEL\n"
        << "Scheduling: Round Robin\n"
        << "Issue width: 1 instruction/cycle\n"
        << "Instructions per warp: "
        << INSTRUCTIONS_PER_WARP << "\n"
        << "Dependency latency: "
        << INSTRUCTION_LATENCY << " cycles\n\n";

    std::cout << std::left
              << std::setw(12) << "Warps"
              << std::setw(15) << "Total Cycles"
              << std::setw(15) << "Issued"
              << std::setw(15) << "Idle Cycles"
              << "Issue Utilization\n";

    std::cout << std::fixed << std::setprecision(2);

    for (int count : {1, 2, 4, 8, 16}) {
        auto result = simulate(
            count,
            INSTRUCTIONS_PER_WARP,
            INSTRUCTION_LATENCY
        );

        double utilization =
            100.0 * result.issuedInstructions /
            result.totalCycles;

        std::cout << std::left
                  << std::setw(12) << count
                  << std::setw(15) << result.totalCycles
                  << std::setw(15) << result.issuedInstructions
                  << std::setw(15) << result.idleCycles
                  << utilization << "%\n";
    }

    return 0;
}
