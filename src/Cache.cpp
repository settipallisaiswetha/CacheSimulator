
#include "Cache.h"
#include <cstdlib>
#include <stdexcept>

// =========================================
// CONSTRUCTOR
// =========================================

Cache::Cache(
    int numberOfSets,
    int associativity,
    ReplacementPolicy replacementPolicy,
    WritePolicy writePolicy,
    Memory& memory
)
    : sets(numberOfSets, CacheSet(associativity)),
      numSets(numberOfSets),
      associativity(associativity),
      replacementPolicy(replacementPolicy),
      writePolicy(writePolicy),
      fifoCounter(0),
      lruCounter(0),
      memory(memory)
{
}

// =========================================
// GETTERS
// =========================================

CacheSet& Cache::getSet(int index) {
    return sets[index];
}

int Cache::getNumberOfSets() const {
    return numSets;
}

int Cache::getAssociativity() const {
    return associativity;
}

// =========================================
// LOOKUP
// =========================================

bool Cache::lookup(
    unsigned long long tag,
    int setIndex
) {
    CacheSet& cacheSet = sets[setIndex];

    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            if (replacementPolicy == ReplacementPolicy::LRU) {
                lruCounter++;
                line.setLastUsedTime(lruCounter);
            }

            return true;
        }
    }

    return false;
}

// =========================================
// GET DATA
// =========================================

bool Cache::getData(
    unsigned long long tag,
    int setIndex,
    unsigned long long& data
) {
    CacheSet& cacheSet = sets[setIndex];

    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            data = line.getData();

            if (replacementPolicy == ReplacementPolicy::LRU) {
                lruCounter++;
                line.setLastUsedTime(lruCounter);
            }

            return true;
        }
    }

    return false;
}

// =========================================
// GET DIRTY DATA
// =========================================

bool Cache::getDirtyData(
    unsigned long long tag,
    int setIndex,
    unsigned long long& blockAddress,
    unsigned long long& data
) {
    CacheSet& cacheSet = sets[setIndex];

    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() &&
            line.getTag() == tag &&
            line.isDirty()) {

            blockAddress = line.getBlockAddress();
            data = line.getData();

            return true;
        }
    }

    return false;
}

// =========================================
// GET EVICTION VICTIM
// =========================================

bool Cache::getEvictionVictim(
    int setIndex,
    unsigned long long& blockAddress,
    unsigned long long& data,
    bool& dirty
) {
    CacheSet& cacheSet = sets[setIndex];

    // No eviction if an empty line exists
    for (int i = 0; i < associativity; i++) {
        if (!cacheSet.getLine(i).isValid()) {
            return false;
        }
    }

    int victimIndex = 0;

    if (replacementPolicy == ReplacementPolicy::FIFO) {
        unsigned long long oldest =
            cacheSet.getLine(0).getInsertionTime();

        for (int i = 1; i < associativity; i++) {
            CacheLine& line = cacheSet.getLine(i);

            if (line.getInsertionTime() < oldest) {
                oldest = line.getInsertionTime();
                victimIndex = i;
            }
        }
    }
    else if (replacementPolicy == ReplacementPolicy::LRU) {
        unsigned long long oldest =
            cacheSet.getLine(0).getLastUsedTime();

        for (int i = 1; i < associativity; i++) {
            CacheLine& line = cacheSet.getLine(i);

            if (line.getLastUsedTime() < oldest) {
                oldest = line.getLastUsedTime();
                victimIndex = i;
            }
        }
    }
    else {
        victimIndex = std::rand() % associativity;
    }

    CacheLine& victim = cacheSet.getLine(victimIndex);

    blockAddress = victim.getBlockAddress();
    data = victim.getData();
    dirty = victim.isDirty();

    return true;
}

// =========================================
// READ
// =========================================

void Cache::read(
    unsigned long long tag,
    int setIndex,
    unsigned long long blockAddress,
    CacheStats& stats
) {
    unsigned long long data = 0;

    if (getData(tag, setIndex, data)) {
        stats.recordHit();
        return;
    }

    stats.recordMiss();

    data = memory.read(blockAddress);

    fillBlock(
        tag,
        setIndex,
        blockAddress,
        data,
        stats
    );
}

// =========================================
// INSERT
// =========================================

void Cache::insert(
    unsigned long long tag,
    int setIndex,
    unsigned long long blockAddress,
    CacheStats& stats
) {
    CacheSet& cacheSet = sets[setIndex];

    // Find an empty line
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (!line.isValid()) {
            line.setTag(tag);
            line.setBlockAddress(blockAddress);
            line.setValid(true);
            line.setDirty(false);
            line.setData(0);

            fifoCounter++;
            line.setInsertionTime(fifoCounter);

            lruCounter++;
            line.setLastUsedTime(lruCounter);

            return;
        }
    }

    // Select replacement victim
    int victimIndex = 0;

    if (replacementPolicy == ReplacementPolicy::FIFO) {
        unsigned long long oldest =
            cacheSet.getLine(0).getInsertionTime();

        for (int i = 1; i < associativity; i++) {
            CacheLine& line = cacheSet.getLine(i);

            if (line.getInsertionTime() < oldest) {
                oldest = line.getInsertionTime();
                victimIndex = i;
            }
        }
    }
    else if (replacementPolicy == ReplacementPolicy::LRU) {
        unsigned long long oldest =
            cacheSet.getLine(0).getLastUsedTime();

        for (int i = 1; i < associativity; i++) {
            CacheLine& line = cacheSet.getLine(i);

            if (line.getLastUsedTime() < oldest) {
                oldest = line.getLastUsedTime();
                victimIndex = i;
            }
        }
    }
    else {
        victimIndex = std::rand() % associativity;
    }

    CacheLine& victim = cacheSet.getLine(victimIndex);

    // Single-level cache write-back
    if (victim.isDirty()) {
        memory.write(
            victim.getBlockAddress(),
            victim.getData()
        );

        stats.recordWriteBack();
    }

    // Replace victim
    victim.setTag(tag);
    victim.setBlockAddress(blockAddress);
    victim.setValid(true);
    victim.setDirty(false);
    victim.setData(0);

    fifoCounter++;
    victim.setInsertionTime(fifoCounter);

    lruCounter++;
    victim.setLastUsedTime(lruCounter);
}

// =========================================
// FILL BLOCK
// =========================================

void Cache::fillBlock(
    unsigned long long tag,
    int setIndex,
    unsigned long long blockAddress,
    unsigned long long data,
    CacheStats& stats
) {
    CacheSet& cacheSet = sets[setIndex];

    // Update existing block
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            line.setData(data);
            line.setDirty(false);

            if (replacementPolicy == ReplacementPolicy::LRU) {
                lruCounter++;
                line.setLastUsedTime(lruCounter);
            }

            return;
        }
    }

    // Allocate new block
    insert(
        tag,
        setIndex,
        blockAddress,
        stats
    );

    // Store data
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            line.setData(data);
            line.setDirty(false);
            return;
        }
    }
}

// =========================================
// WRITE
// =========================================

void Cache::write(
    unsigned long long tag,
    int setIndex,
    unsigned long long blockAddress,
    unsigned long long data,
    CacheStats& stats
) {
    CacheSet& cacheSet = sets[setIndex];

    // Write hit
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            stats.recordHit();
            line.setData(data);

            if (writePolicy == WritePolicy::WRITE_BACK) {
                line.setDirty(true);
            }
            else {
                memory.write(blockAddress, data);
                line.setDirty(false);
            }

            if (replacementPolicy == ReplacementPolicy::LRU) {
                lruCounter++;
                line.setLastUsedTime(lruCounter);
            }

            return;
        }
    }

    // Write miss: write allocate
    stats.recordMiss();

    insert(
        tag,
        setIndex,
        blockAddress,
        stats
    );

    // Update newly allocated block
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            line.setData(data);

            if (writePolicy == WritePolicy::WRITE_BACK) {
                line.setDirty(true);
            }
            else {
                memory.write(blockAddress, data);
                line.setDirty(false);
            }

            return;
        }
    }
}

// =========================================
// EVICT BLOCK FOR CACHE HIERARCHY
// =========================================

bool Cache::evictBlock(
    int setIndex,
    unsigned long long& blockAddress,
    unsigned long long& data,
    bool& dirty
) {
    CacheSet& cacheSet = sets[setIndex];

    int victimIndex = -1;

    if (replacementPolicy == ReplacementPolicy::RANDOM) {
        int validCount = 0;

        for (int i = 0; i < associativity; i++) {
            if (cacheSet.getLine(i).isValid()) {
                validCount++;
            }
        }

        if (validCount == 0) {
            return false;
        }

        int selected = std::rand() % validCount;

        for (int i = 0; i < associativity; i++) {
            if (cacheSet.getLine(i).isValid()) {
                if (selected == 0) {
                    victimIndex = i;
                    break;
                }
                selected--;
            }
        }
    }
    else {
        unsigned long long oldest = 0;

        for (int i = 0; i < associativity; i++) {
            CacheLine& line = cacheSet.getLine(i);

            if (!line.isValid()) {
                continue;
            }

            unsigned long long timestamp =
                replacementPolicy == ReplacementPolicy::FIFO
                    ? line.getInsertionTime()
                    : line.getLastUsedTime();

            if (victimIndex == -1 || timestamp < oldest) {
                oldest = timestamp;
                victimIndex = i;
            }
        }
    }

    if (victimIndex == -1) {
        return false;
    }

    CacheLine& victim = cacheSet.getLine(victimIndex);

    blockAddress = victim.getBlockAddress();
    data = victim.getData();
    dirty = victim.isDirty();

    // Remove block without updating RAM
    victim.setValid(false);
    victim.setDirty(false);

    return true;
}

// =========================================
// FILL BLOCK FOR CACHE HIERARCHY
// =========================================

void Cache::fillBlockForHierarchy(
    unsigned long long tag,
    int setIndex,
    unsigned long long blockAddress,
    unsigned long long data,
    bool dirty,
    CacheStats& stats
) {
    CacheSet& cacheSet = sets[setIndex];

    // Update existing block
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            line.setData(data);
            line.setDirty(dirty);

            if (replacementPolicy == ReplacementPolicy::LRU) {
                lruCounter++;
                line.setLastUsedTime(lruCounter);
            }

            return;
        }
    }

    // Find a free line
    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (!line.isValid()) {
            line.setTag(tag);
            line.setBlockAddress(blockAddress);
            line.setData(data);
            line.setValid(true);
            line.setDirty(dirty);

            fifoCounter++;
            line.setInsertionTime(fifoCounter);

            lruCounter++;
            line.setLastUsedTime(lruCounter);

            return;
        }
    }

    throw std::runtime_error(
        "Cache set full: hierarchy must evict a victim first"
    );
}

// =========================================
// INVALIDATE CACHE BLOCK - NEW
// =========================================

bool Cache::invalidateBlock(
    unsigned long long tag,
    int setIndex,
    unsigned long long& data,
    bool& dirty
) {
    CacheSet& cacheSet = sets[setIndex];

    for (int i = 0; i < associativity; i++) {
        CacheLine& line = cacheSet.getLine(i);

        if (line.isValid() && line.getTag() == tag) {
            data = line.getData();
            dirty = line.isDirty();

            line.setValid(false);
            line.setDirty(false);

            return true;
        }
    }

    return false;
}
