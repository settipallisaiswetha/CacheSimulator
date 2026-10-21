#include "Memory.h"

Memory::Memory() {
}


// =========================================================
// WRITE TO MEMORY
// =========================================================

void Memory::write(
    unsigned long long address,
    unsigned long long value
) {
    data[address] = value;
}


// =========================================================
// READ FROM MEMORY
// =========================================================

unsigned long long Memory::read(
    unsigned long long address
) {

    if (data.find(address) != data.end()) {
        return data[address];
    }

    // If address has never been written,
    // return 0 as the default value.

    return 0;
}


// =========================================================
// CHECK WHETHER ADDRESS EXISTS
// =========================================================

bool Memory::contains(
    unsigned long long address
) const {

    return data.find(address) != data.end();
}


// =========================================================
// NUMBER OF STORED MEMORY LOCATIONS
// =========================================================

int Memory::getSize() const {

    return static_cast<int>(data.size());
}