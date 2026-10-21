#ifndef MEMORY_H
#define MEMORY_H

#include <unordered_map>

class Memory {
private:
    std::unordered_map<unsigned long long, unsigned long long> data;

public:
    Memory();

    void write(
        unsigned long long address,
        unsigned long long value
    );

    unsigned long long read(
        unsigned long long address
    );

    bool contains(
        unsigned long long address
    ) const;

    int getSize() const;
};

#endif