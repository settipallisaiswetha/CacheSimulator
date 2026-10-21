
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

// Matrix multiplication: C = A * B
constexpr int N = 32;
constexpr unsigned long long ELEMENT_SIZE = 8;

// Separate address regions for A, B and C.
constexpr unsigned long long A_BASE = 0;
constexpr unsigned long long B_BASE = 1048576;
constexpr unsigned long long C_BASE = 2097152;

// Row-major matrix addressing.
unsigned long long address(
    unsigned long long base,
    int row,
    int col
) {
    return base +
        (static_cast<unsigned long long>(row) * N + col)
        * ELEMENT_SIZE;
}

void readElement(
    std::ofstream& file,
    unsigned long long addr
) {
    file << "R " << addr << "\n";
}

void writeElement(
    std::ofstream& file,
    unsigned long long addr,
    unsigned long long value
) {
    file << "W " << addr << " " << value << "\n";
}

// =======================================
// NAIVE MATRIX MULTIPLICATION
// =======================================

void generateNaive() {
    std::ofstream file("traces/matrix_naive.txt");

    if (!file) {
        throw std::runtime_error("Cannot create naive trace");
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            for (int k = 0; k < N; ++k) {
                readElement(file, address(A_BASE, i, k));
                readElement(file, address(B_BASE, k, j));
            }

            writeElement(
                file,
                address(C_BASE, i, j),
                0
            );
        }
    }

    std::cout << "Generated matrix_naive.txt\n";
}

// =======================================
// TILED MATRIX MULTIPLICATION
// =======================================

void generateTiled(int tileSize) {
    const std::string filename =
        "traces/matrix_tiled_" +
        std::to_string(tileSize) + ".txt";

    std::ofstream file(filename);

    if (!file) {
        throw std::runtime_error(
            "Cannot create tiled trace"
        );
    }

    // C is initialized before accumulation.
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            writeElement(
                file,
                address(C_BASE, i, j),
                0
            );
        }
    }

    for (int ii = 0; ii < N; ii += tileSize) {
        for (int jj = 0; jj < N; jj += tileSize) {
            for (int kk = 0; kk < N; kk += tileSize) {

                for (int i = ii; i < ii + tileSize; ++i) {
                    for (int j = jj; j < jj + tileSize; ++j) {
                        // Read current partial result.
                        readElement(
                            file,
                            address(C_BASE, i, j)
                        );

                        for (int k = kk; k < kk + tileSize; ++k) {
                            readElement(
                                file,
                                address(A_BASE, i, k)
                            );

                            readElement(
                                file,
                                address(B_BASE, k, j)
                            );
                        }

                        // Write updated partial result.
                        writeElement(
                            file,
                            address(C_BASE, i, j),
                            0
                        );
                    }
                }
            }
        }
    }

    std::cout << "Generated " << filename << "\n";
}

int main() {
    try {
        fs::create_directories("traces");

        generateNaive();
        generateTiled(4);
        generateTiled(8);
        generateTiled(16);

        std::cout
            << "\nAll matrix workload traces generated.\n";

        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "ERROR: "
                  << error.what() << "\n";
        return 1;
    }
}
