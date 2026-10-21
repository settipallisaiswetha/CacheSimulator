#ifndef CACHE_LINE_H
#define CACHE_LINE_H

class CacheLine {
private:
    bool valid;
    bool dirty;

    // Identifies the memory block
    unsigned long long blockAddress;

    // Tag of the memory block
    unsigned long long tag;

    // Actual data stored in the cache line
    unsigned long long data;

    // FIFO tracking
    unsigned long long insertionTime;

    // LRU tracking
    unsigned long long lastUsedTime;

public:
    CacheLine();

    bool isValid() const;
    bool isDirty() const;

    unsigned long long getBlockAddress() const;
    unsigned long long getTag() const;
    unsigned long long getData() const;

    unsigned long long getInsertionTime() const;
    unsigned long long getLastUsedTime() const;

    void setValid(bool value);
    void setDirty(bool value);

    void setBlockAddress(
        unsigned long long value
    );

    void setTag(
        unsigned long long value
    );

    void setData(
        unsigned long long value
    );

    void setInsertionTime(
        unsigned long long value
    );

    void setLastUsedTime(
        unsigned long long value
    );
};

#endif