
#include <iostream>
#include <vector>
#include <array>
#include <iomanip>
#include <stdexcept>

enum class Type {
    ALU,
    L1,
    L2,
    DRAM
};

struct Instruction {
    Type type;
    int latency;
};

struct Warp {
    int nextInstruction = 0;
    int readyCycle = 0;
    Type waitingOn = Type::ALU;
};

struct Result {
    int cycles = 0;
    int issued = 0;
    int idle = 0;
    std::array<int, 4> stallCycles{};
};

int typeIndex(Type type) {
    return static_cast<int>(type);
}

Result simulate(
    int numWarps,
    const std::vector<Instruction>& program
) {
    if (numWarps <= 0) {
        throw std::invalid_argument("numWarps must be positive");
    }

    for (const auto& instruction : program) {
        if (instruction.latency <= 0) {
            throw std::invalid_argument(
                "Instruction latency must be positive"
            );
        }
    }

    std::vector<Warp> warps(numWarps);
    Result result;
    int nextWarp = 0;

    while (true) {
        bool unfinished = false;

        for (const auto& warp : warps) {
            if (warp.nextInstruction <
                static_cast<int>(program.size())) {
                unfinished = true;
                break;
            }
        }

        if (!unfinished) {
            break;
        }

        bool issued = false;

        for (int offset = 0; offset < numWarps; ++offset) {
            int index = (nextWarp + offset) % numWarps;
            Warp& warp = warps[index];

            if (warp.nextInstruction >=
                static_cast<int>(program.size())) {
                continue;
            }

            if (warp.readyCycle > result.cycles) {
                continue;
            }

            const Instruction& instruction =
                program[warp.nextInstruction];

            warp.readyCycle =
                result.cycles + instruction.latency;

            warp.waitingOn = instruction.type;

            ++warp.nextInstruction;
            ++result.issued;

            nextWarp = (index + 1) % numWarps;
            issued = true;
            break;
        }

        if (!issued) {
            ++result.idle;

            // Attribute the idle cycle to the dependency
            // with the earliest upcoming ready time.
            // This is a heuristic, not hardware stall attribution.
            int earliestReady = -1;
            Type reason = Type::ALU;

            for (const auto& warp : warps) {
                if (warp.nextInstruction >=
                    static_cast<int>(program.size())) {
                    continue;
                }

                if (earliestReady == -1 ||
                    warp.readyCycle < earliestReady) {
                    earliestReady = warp.readyCycle;
                    reason = warp.waitingOn;
                }
            }

            ++result.stallCycles[typeIndex(reason)];
        }

        ++result.cycles;
    }

    return result;
}

int main() {
    const std::vector<Instruction> program = {
        {Type::ALU, 4},
        {Type::ALU, 4},
        {Type::L1, 20},
        {Type::ALU, 4},
        {Type::L2, 100},
        {Type::ALU, 4},
        {Type::DRAM, 400},
        {Type::ALU, 4}
    };

    std::cout << "\nGPU SCHEDULER STALL ANALYSIS\n";
    std::cout << "Scheduling: Round Robin\n";
    std::cout << "Issue width: 1 instruction/cycle\n\n";

    std::cout << std::left
              << std::setw(8) << "Warps"
              << std::setw(10) << "Cycles"
              << std::setw(10) << "Idle"
              << std::setw(10) << "ALU"
              << std::setw(10) << "L1"
              << std::setw(10) << "L2"
              << std::setw(10) << "DRAM"
              << "Utilization\n";

    std::cout << std::fixed << std::setprecision(2);

    for (int count : {1, 2, 4, 8, 16, 32}) {
        Result r = simulate(count, program);

        double utilization =
            100.0 * r.issued / r.cycles;

        std::cout << std::left
                  << std::setw(8) << count
                  << std::setw(10) << r.cycles
                  << std::setw(10) << r.idle
                  << std::setw(10) << r.stallCycles[0]
                  << std::setw(10) << r.stallCycles[1]
                  << std::setw(10) << r.stallCycles[2]
                  << std::setw(10) << r.stallCycles[3]
                  << utilization << "%\n";
    }

    return 0;
}
