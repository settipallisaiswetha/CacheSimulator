
#ifndef CACHE_HIERARCHY_H
#define CACHE_HIERARCHY_H

#include "Cache.h"
#include "CacheStats.h"
#include "Memory.h"
#include "ReplacementPolicy.h"
#include "WritePolicy.h"

class CacheHierarchy {

private:

    // =====================================
    // CACHE LEVELS
    // =====================================

    Cache l1;
    Cache l2;

    // =====================================
    // CACHE STATISTICS
    // =====================================

    CacheStats l1Stats;
    CacheStats l2Stats;

    // =====================================
    // MAIN MEMORY
    // =====================================

    Memory& memory;

    // =====================================
    // WRITE POLICY
    // =====================================

    WritePolicy writePolicy;

    // =====================================
    // INSTALL BLOCK IN L1
    // =====================================

    void installL1(
        unsigned long long blockAddress,
        unsigned long long data,
        bool dirty
    );

    // =====================================
    // INSTALL BLOCK IN L2
    // =====================================

    void installL2(
        unsigned long long blockAddress,
        unsigned long long data,
        bool dirty
    );

public:

    // =====================================
    // CONSTRUCTOR
    // =====================================

    CacheHierarchy(
        int l1Sets,
        int l1Associativity,
        int l2Sets,
        int l2Associativity,
        ReplacementPolicy replacementPolicy,
        WritePolicy writePolicy,
        Memory& memory
    );

    // =====================================
    // READ OPERATION
    // =====================================

    void read(
        unsigned long long address,
        int blockSize
    );

    // =====================================
    // WRITE OPERATION
    // =====================================

    void write(
        unsigned long long address,
        unsigned long long data,
        int blockSize
    );

    // =====================================
    // GET L1 STATISTICS
    // =====================================

    const CacheStats& getL1Stats() const;

    // =====================================
    // GET L2 STATISTICS
    // =====================================

    const CacheStats& getL2Stats() const;
};

#endif
