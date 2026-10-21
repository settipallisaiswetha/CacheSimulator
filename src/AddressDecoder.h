#ifndef ADDRESS_DECODER_H
#define ADDRESS_DECODER_H

class AddressDecoder {
private:
    int blockSize;
    int numberOfSets;

public:
    AddressDecoder(int blockSize, int numberOfSets);

    unsigned long long getOffset(unsigned long long address) const;
    unsigned long long getIndex(unsigned long long address) const;
    unsigned long long getTag(unsigned long long address) const;
};

#endif