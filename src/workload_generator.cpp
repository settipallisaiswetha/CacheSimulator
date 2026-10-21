
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

constexpr int BLOCK_SIZE = 64;
constexpr int OPERATIONS = 1000;

// Write one memory operation into the trace.
void writeOperation(
    std::ofstream& file,
    unsigned long long address,
    int index
) {
    // 20% writes and 80% reads
    if (index % 5 == 0) {
        file << "W " << address
             << " " << (index + 100)
             << "\n";
    } else {
        file << "R " << address << "\n";
    }
}

// ==========================================
// WORKLOAD 1: SEQUENTIAL ACCESS
// ==========================================

void generateSequential() {
    std::ofstream file("traces/sequential.txt");

    if (!file) {
        throw std::runtime_error(
            "Cannot create sequential trace"
        );
    }

    for (int i = 0; i < OPERATIONS; i++) {
        unsigned long long address =
            static_cast<unsigned long long>(i) *
            BLOCK_SIZE;

        writeOperation(file, address, i);
    }

    std::cout << "Generated sequential.txt\n";
}

// ==========================================
// WORKLOAD 2: RANDOM ACCESS
// ==========================================

void generateRandom() {
    std::ofstream file("traces/random.txt");

    if (!file) {
        throw std::runtime_error(
            "Cannot create random trace"
        );
    }

    std::mt19937 generator(42);

    // Random selection among 4096 memory blocks
    std::uniform_int_distribution<int> distribution(
        0, 4095
    );

    for (int i = 0; i < OPERATIONS; i++) {
        unsigned long long block =
            distribution(generator);

        unsigned long long address =
            block * BLOCK_SIZE;

        writeOperation(file, address, i);
    }

    std::cout << "Generated random.txt\n";
}

// ==========================================
// WORKLOAD 3: REPEATED ACCESS
// ==========================================

void generateRepeated() {
    std::ofstream file("traces/repeated.txt");

    if (!file) {
        throw std::runtime_error(
            "Cannot create repeated trace"
        );
    }

    // Repeatedly access only 8 memory blocks
    for (int i = 0; i < OPERATIONS; i++) {
        unsigned long long block =
            static_cast<unsigned long long>(i % 8);

        unsigned long long address =
            block * BLOCK_SIZE;

        writeOperation(file, address, i);
    }

    std::cout << "Generated repeated.txt\n";
}

// ==========================================
// MAIN
// ==========================================

int main() {
    try {
        fs::create_directories("traces");

        generateSequential();
        generateRandom();
        generateRepeated();

        std::cout
            << "\nSuccessfully generated "
            << OPERATIONS
            << " operations per workload.\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr
            << "ERROR: "
            << error.what()
            << "\n";

        return 1;
    }
}
