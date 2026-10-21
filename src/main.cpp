#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <cmath>

#include "Cache.h"
#include "AddressDecoder.h"
#include "CacheStats.h"
#include "Memory.h"

int main() {

    // =========================================
    // STEP 1: USER CONFIGURATION
    // =========================================

    int blockSize;
    int numberOfSets;
    int associativity;
    int replacementChoice;
    int writeChoice;

    std::cout << "Enter Block Size: ";
    std::cin >> blockSize;

    std::cout << "Enter Number of Sets: ";
    std::cin >> numberOfSets;

    std::cout << "Enter Associativity: ";
    std::cin >> associativity;

    std::cout << "\nSelect Replacement Policy:\n";
    std::cout << "1. FIFO\n";
    std::cout << "2. LRU\n";
    std::cout << "3. RANDOM\n";
    std::cout << "Enter choice: ";
    std::cin >> replacementChoice;

    std::cout << "\nSelect Write Policy:\n";
    std::cout << "1. WRITE_BACK\n";
    std::cout << "2. WRITE_THROUGH\n";
    std::cout << "Enter choice: ";
    std::cin >> writeChoice;

    // =========================================
    // STEP 2: VALIDATE CONFIGURATION
    // =========================================

    if (!std::cin ||
        blockSize <= 0 ||
        (blockSize & (blockSize - 1)) != 0 ||
        numberOfSets <= 0 ||
        associativity <= 0 ||
        replacementChoice < 1 ||
        replacementChoice > 3 ||
        writeChoice < 1 ||
        writeChoice > 2) {

        std::cerr << "Invalid cache configuration!\n";
        return 1;
    }

    // =========================================
    // STEP 3: SELECT REPLACEMENT POLICY
    // =========================================

    ReplacementPolicy replacementPolicy =
        ReplacementPolicy::LRU;

    if (replacementChoice == 1) {
        replacementPolicy = ReplacementPolicy::FIFO;
    }
    else if (replacementChoice == 3) {
        replacementPolicy = ReplacementPolicy::RANDOM;
    }

    // =========================================
    // STEP 4: SELECT WRITE POLICY
    // =========================================

    WritePolicy writePolicy =
        WritePolicy::WRITE_BACK;

    if (writeChoice == 2) {
        writePolicy = WritePolicy::WRITE_THROUGH;
    }

    // =========================================
    // STEP 5: CONFIGURABLE CACHE LATENCY
    // =========================================

    double hitLatency;
    double missPenalty;

    std::cout << "\nEnter Cache Hit Latency (cycles): ";
    std::cin >> hitLatency;

    std::cout << "Enter Memory Miss Penalty (cycles): ";
    std::cin >> missPenalty;

    if (!std::cin ||
        !std::isfinite(hitLatency) ||
        !std::isfinite(missPenalty) ||
        hitLatency < 0 ||
        missPenalty < 0) {

        std::cerr << "Invalid latency values!\n";
        return 1;
    }

    // =========================================
    // STEP 6: DISPLAY CONFIGURATION
    // =========================================

    std::cout << "\nReplacement Policy: ";

    if (replacementPolicy == ReplacementPolicy::FIFO) {
        std::cout << "FIFO\n";
    }
    else if (replacementPolicy == ReplacementPolicy::LRU) {
        std::cout << "LRU\n";
    }
    else {
        std::cout << "RANDOM\n";
    }

    std::cout << "Write Policy: ";

    if (writePolicy == WritePolicy::WRITE_BACK) {
        std::cout << "WRITE_BACK\n";
    }
    else {
        std::cout << "WRITE_THROUGH\n";
    }

    std::cout << "Hit Latency: "
              << hitLatency << " cycles\n";

    std::cout << "Miss Penalty: "
              << missPenalty << " cycles\n";

    // =========================================
    // STEP 7: INITIALIZE CACHE AND MEMORY
    // =========================================

    AddressDecoder decoder(
        blockSize,
        numberOfSets
    );

    Memory memory;

    // Preload data for address 1512.
    // This is for our current demonstration trace.

    unsigned long long preloadBlock =
        1512ULL /
        static_cast<unsigned long long>(blockSize);

    memory.write(preloadBlock, 900);

    Cache cache(
        numberOfSets,
        associativity,
        replacementPolicy,
        writePolicy,
        memory
    );

    CacheStats stats;

    // =========================================
    // STEP 8: OPEN TRACE FILE
    // =========================================

    std::ifstream traceFile("trace.txt");

    if (!traceFile.is_open()) {
        std::cerr << "Error: Cannot open trace.txt\n";
        return 1;
    }

    std::cout << "\nReading memory access trace...\n";

    // =========================================
    // STEP 9: PROCESS TRACE OPERATIONS
    // =========================================

    char operation;
    unsigned long long address;
    unsigned long long data;

    while (traceFile >> operation >> address) {

        unsigned long long blockAddress =
            address /
            static_cast<unsigned long long>(blockSize);

        unsigned long long index =
            decoder.getIndex(address);

        unsigned long long tag =
            decoder.getTag(address);

        // -------------------------------------
        // WRITE OPERATION
        // -------------------------------------

        if (operation == 'W') {

            if (!(traceFile >> data)) {
                std::cerr
                    << "Invalid write operation in trace.\n";
                return 1;
            }

            std::cout << "\nWRITE Address: "
                      << address
                      << " | Block: " << blockAddress
                      << " | Index: " << index
                      << " | Tag: " << tag
                      << " | Data: " << data
                      << "\n";

            cache.write(
                tag,
                static_cast<int>(index),
                blockAddress,
                data,
                stats
            );
        }

        // -------------------------------------
        // READ OPERATION
        // -------------------------------------

        else if (operation == 'R') {

            std::cout << "\nREAD Address: "
                      << address
                      << " | Block: " << blockAddress
                      << " | Index: " << index
                      << " | Tag: " << tag
                      << "\n";

            cache.read(
                tag,
                static_cast<int>(index),
                blockAddress,
                stats
            );
        }

        else {
            std::cerr << "Unknown operation: "
                      << operation << "\n";
            return 1;
        }
    }

    traceFile.close();

    // =========================================
    // STEP 10: DISPLAY FINAL CACHE CONTENTS
    // =========================================

    std::cout << "\nFinal Cache Contents:\n";

    for (int s = 0;
         s < cache.getNumberOfSets();
         s++) {

        CacheSet& cacheSet = cache.getSet(s);

        for (int i = 0;
             i < cache.getAssociativity();
             i++) {

            CacheLine& line =
                cacheSet.getLine(i);

            if (line.isValid()) {

                std::cout << "Set " << s
                          << " | Line " << i
                          << " | Block: "
                          << line.getBlockAddress()
                          << " | Tag: "
                          << line.getTag()
                          << " | Data: "
                          << line.getData()
                          << " | Dirty: "
                          << line.isDirty()
                          << "\n";
            }
        }
    }

    // =========================================
    // STEP 11: CACHE STATISTICS
    // =========================================

    std::cout << "\nCache Statistics:\n";

    std::cout << "Accesses: "
              << stats.getAccesses()
              << "\n";

    std::cout << "Hits: "
              << stats.getHits()
              << "\n";

    std::cout << "Misses: "
              << stats.getMisses()
              << "\n";

    std::cout << "Hit Rate: "
              << stats.getHitRate()
              << "\n";

    std::cout << "Miss Rate: "
              << stats.getMissRate()
              << "\n";

    std::cout << "Write-backs: "
              << stats.getWriteBacks()
              << "\n";

    // =========================================
    // STEP 12: CACHE LATENCY MODELLING
    // =========================================

    double missRate =
        stats.getMissRate();

    // AMAT formula:
    // AMAT = Hit Latency + Miss Rate * Miss Penalty

    double amat =
        hitLatency +
        missRate * missPenalty;

    // Simplified total memory access cycles

    double totalCycles =
        stats.getAccesses() * hitLatency +
        stats.getMisses() * missPenalty;

    std::cout << "\nCache Performance:\n";

    std::cout << "Hit Latency: "
              << hitLatency
              << " cycles\n";

    std::cout << "Miss Penalty: "
              << missPenalty
              << " cycles\n";

    std::cout << "Average Memory Access Time: "
              << amat
              << " cycles\n";

    std::cout << "Total Memory Access Cycles: "
              << totalCycles
              << " cycles\n";

    // =========================================
    // STEP 13: DISPLAY MAIN MEMORY
    // =========================================

    std::cout << "\nMemory:\n";

    std::cout << "Stored Blocks: "
              << memory.getSize()
              << "\n";

    std::cout << "Block for Address 1000: "
              << memory.read(
                     1000ULL /
                     static_cast<unsigned long long>(blockSize)
                 )
              << "\n";

    std::cout << "Block for Address 1256: "
              << memory.read(
                     1256ULL /
                     static_cast<unsigned long long>(blockSize)
                 )
              << "\n";

    std::cout << "Block for Address 1512: "
              << memory.read(preloadBlock)
              << "\n";

    return 0;
}