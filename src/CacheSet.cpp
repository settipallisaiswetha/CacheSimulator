#include "CacheSet.h"

CacheSet::CacheSet(int associativity)
    : lines(associativity) {
}

CacheLine& CacheSet::getLine(int index) {
    return lines[index];
}

int CacheSet::getAssociativity() const {
    return static_cast<int>(lines.size());
}