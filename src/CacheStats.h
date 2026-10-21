
#ifndef CACHE_STATS_H
#define CACHE_STATS_H

class CacheStats {
private:
    unsigned long long accesses;
    unsigned long long hits;
    unsigned long long misses;
    unsigned long long writeBacks;

public:
    CacheStats();

    void recordHit();
    void recordMiss();
    void recordWriteBack();

    unsigned long long getAccesses() const;
    unsigned long long getHits() const;
    unsigned long long getMisses() const;
    unsigned long long getWriteBacks() const;

    double getHitRate() const;
    double getMissRate() const;
};

#endif

