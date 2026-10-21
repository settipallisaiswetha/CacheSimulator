
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

constexpr unsigned long long BLOCK_SIZE = 64;
constexpr int OPERATIONS = 1000;
constexpr int WORKING_SET_BLOCKS = 256;

// Generate memory accesses separated by a fixed stride.
// The address wraps around the working set.
void generateStride(
    const std::string& filename,
    int strideBlocks
) {
    std::ofstream file(filename);

    if (!file) {
        throw std::runtime_error(
            "Cannot create trace: " + filename
        );
    }

    for (int i = 0; i < OPERATIONS; ++i) {
        unsigned long long block =
            (static_cast<unsigned long long>(i) *
             strideBlocks) % WORKING_SET_BLOCKS;

        unsigned long long address =
            block * BLOCK_SIZE;

        if (i % 5 == 0) {
            file << "W " << address
                 << " " << (100 + i) << "\n";
        } else {
            file << "R " << address << "\n";
        }
    }

    std::cout << "Generated " << filename << "\n";
}

int main() {
    try {
        fs::create_directories("traces");

        generateStride("traces/stride_1.txt", 1);
        generateStride("traces/stride_2.txt", 2);
        generateStride("traces/stride_4.txt", 4);
        generateStride("traces/stride_8.txt", 8);

        std::cout
            << "\nGenerated four stride workloads "
            << "with 1000 operations each.\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "ERROR: "
                  << error.what() << "\n";
        return 1;
    }
}
