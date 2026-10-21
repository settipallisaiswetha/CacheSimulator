#include "CacheStats.h"

CacheStats::CacheStats()
    : accesses(0),
      hits(0),
      misses(0),
      writeBacks(0) {
}

void CacheStats::recordHit() {
    accesses++;
    hits++;
}

void CacheStats::recordMiss() {
    accesses++;
    misses++;
}

void CacheStats::recordWriteBack() {
    writeBacks++;
}

unsigned long long CacheStats::getAccesses() const {
    return accesses;
}

unsigned long long CacheStats::getHits() const {
    return hits;
}

unsigned long long CacheStats::getMisses() const {
    return misses;
}

unsigned long long CacheStats::getWriteBacks() const {
    return writeBacks;
}

double CacheStats::getHitRate() const {
    if (accesses == 0) {
        return 0.0;
    }

    return static_cast<double>(hits) / accesses;
}

double CacheStats::getMissRate() const {
    if (accesses == 0) {
        return 0.0;
    }

    return static_cast<double>(misses) / accesses;
}