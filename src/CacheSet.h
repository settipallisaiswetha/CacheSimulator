#ifndef CACHE_SET_H
#define CACHE_SET_H

#include <vector>
#include "CacheLine.h"

class CacheSet {
private:
    std::vector<CacheLine> lines;

public:
    CacheSet(int associativity);

    CacheLine& getLine(int index);
    int getAssociativity() const;
};

#endif