
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

enum class InstructionType {
    ALU,
    L1,
    L2,
    DRAM
};

struct Instruction {
    InstructionType type;
    int latency;
};

struct Warp {
    int id;
    int nextInstruction = 0;
    int readyCycle = 0;
};

struct Result {
    int cycles = 0;
    int issued = 0;
    int idle = 0;
};

Result simulate(
    int numWarps,
    const std::vector<Instruction>& program
) {
    std::vector<Warp> warps;

    for (int i = 0; i < numWarps; ++i) {
        warps.push_back({i, 0, 0});
    }

    Result result;
    int nextWarp = 0;

    while (true) {
        bool finished = true;

        for (const auto& warp : warps) {
            if (warp.nextInstruction <
                static_cast<int>(program.size())) {
                finished = false;
                break;
            }
        }

        if (finished) break;

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

            ++warp.nextInstruction;
            ++result.issued;

            nextWarp = (index + 1) % numWarps;
            issued = true;
            break;
        }

        if (!issued) {
            ++result.idle;
        }

        ++result.cycles;
    }

    return result;
}

int main() {
    const std::vector<Instruction> program = {
        {InstructionType::ALU, 4},
        {InstructionType::ALU, 4},
        {InstructionType::L1, 20},
        {InstructionType::ALU, 4},
        {InstructionType::L2, 100},
        {InstructionType::ALU, 4},
        {InstructionType::DRAM, 400},
        {InstructionType::ALU, 4}
    };

    std::cout << "\nGPU VARIABLE LATENCY SCHEDULER\n";
    std::cout << "Scheduling: Round Robin\n";
    std::cout << "Issue width: 1 instruction/cycle\n";
    std::cout << "ALU: 4 | L1: 20 | L2: 100 | DRAM: 400\n\n";

    std::cout << std::left
              << std::setw(10) << "Warps"
              << std::setw(15) << "Cycles"
              << std::setw(15) << "Issued"
              << std::setw(15) << "Idle"
              << "Utilization\n";

    std::cout << std::fixed << std::setprecision(2);

    for (int count : {1, 2, 4, 8, 16, 32}) {
        Result r = simulate(count, program);

        double utilization =
            100.0 * r.issued / r.cycles;

        std::cout << std::left
                  << std::setw(10) << count
                  << std::setw(15) << r.cycles
                  << std::setw(15) << r.issued
                  << std::setw(15) << r.idle
                  << utilization << "%\n";
    }

    return 0;
}
