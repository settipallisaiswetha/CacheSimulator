
#ifndef CACHE_H
#define CACHE_H

#include <vector>

#include "CacheSet.h"
#include "ReplacementPolicy.h"
#include "WritePolicy.h"
#include "CacheStats.h"
#include "Memory.h"

class Cache {

private:

    // Cache organization
    std::vector<CacheSet> sets;

    int numSets;
    int associativity;

    // Cache policies
    ReplacementPolicy replacementPolicy;
    WritePolicy writePolicy;

    // Replacement tracking
    unsigned long long fifoCounter;
    unsigned long long lruCounter;

    // Main memory reference
    Memory& memory;

public:

    // =====================================
    // CONSTRUCTOR
    // =====================================

    Cache(
        int numberOfSets,
        int associativity,
        ReplacementPolicy replacementPolicy,
        WritePolicy writePolicy,
        Memory& memory
    );

    // =====================================
    // GETTERS
    // =====================================

    CacheSet& getSet(int index);

    int getNumberOfSets() const;

    int getAssociativity() const;

    // =====================================
    // CACHE LOOKUP
    // =====================================

    bool lookup(
        unsigned long long tag,
        int setIndex
    );

    bool getData(
        unsigned long long tag,
        int setIndex,
        unsigned long long& data
    );

    // =====================================
    // DIRTY DATA DETECTION
    // =====================================

    bool getDirtyData(
        unsigned long long tag,
        int setIndex,
        unsigned long long& blockAddress,
        unsigned long long& data
    );

    bool invalidateBlock(
    unsigned long long tag,
    int setIndex,
    unsigned long long& data,
    bool& dirty
);

    // =====================================
    // EVICTION VICTIM INSPECTION
    // =====================================

    bool getEvictionVictim(
        int setIndex,
        unsigned long long& blockAddress,
        unsigned long long& data,
        bool& dirty
    );

    // =====================================
    // READ OPERATION
    // =====================================

    void read(
        unsigned long long tag,
        int setIndex,
        unsigned long long blockAddress,
        CacheStats& stats
    );

    // =====================================
    // CACHE INSERTION
    // =====================================

    void insert(
        unsigned long long tag,
        int setIndex,
        unsigned long long blockAddress,
        CacheStats& stats
    );

    // =====================================
    // CACHE BLOCK FILL
    // =====================================

    void fillBlock(
        unsigned long long tag,
        int setIndex,
        unsigned long long blockAddress,
        unsigned long long data,
        CacheStats& stats
    );

    // =====================================
    // WRITE OPERATION
    // =====================================

    void write(
        unsigned long long tag,
        int setIndex,
        unsigned long long blockAddress,
        unsigned long long data,
        CacheStats& stats
    );

    // =====================================
    // NEW: HIERARCHY-AWARE EVICTION
    // =====================================

    bool evictBlock(
        int setIndex,
        unsigned long long& blockAddress,
        unsigned long long& data,
        bool& dirty
    );

    // =====================================
    // NEW: HIERARCHY-AWARE BLOCK FILL
    // =====================================

    void fillBlockForHierarchy(
        unsigned long long tag,
        int setIndex,
        unsigned long long blockAddress,
        unsigned long long data,
        bool dirty,
        CacheStats& stats
    );
};

#endif
