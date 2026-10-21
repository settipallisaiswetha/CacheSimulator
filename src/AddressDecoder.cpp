#include "AddressDecoder.h"

AddressDecoder::AddressDecoder(int blockSize, int numberOfSets)
    : blockSize(blockSize),
      numberOfSets(numberOfSets) {
}

unsigned long long AddressDecoder::getOffset(
    unsigned long long address) const {

    return address & (blockSize - 1);
}

unsigned long long AddressDecoder::getIndex(
    unsigned long long address) const {

    return (address / blockSize) % numberOfSets;
}

unsigned long long AddressDecoder::getTag(
    unsigned long long address) const {

    return address / (static_cast<unsigned long long>(blockSize) * numberOfSets);
}