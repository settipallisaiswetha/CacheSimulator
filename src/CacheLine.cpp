#include "CacheLine.h"

CacheLine::CacheLine()
    : valid(false),
      dirty(false),
      blockAddress(0),
      tag(0),
      data(0),
      insertionTime(0),
      lastUsedTime(0) {
}

bool CacheLine::isValid() const {
    return valid;
}

bool CacheLine::isDirty() const {
    return dirty;
}

unsigned long long CacheLine::getBlockAddress() const {
    return blockAddress;
}

unsigned long long CacheLine::getTag() const {
    return tag;
}

unsigned long long CacheLine::getData() const {
    return data;
}

unsigned long long CacheLine::getInsertionTime() const {
    return insertionTime;
}

unsigned long long CacheLine::getLastUsedTime() const {
    return lastUsedTime;
}

void CacheLine::setValid(bool value) {
    valid = value;
}

void CacheLine::setDirty(bool value) {
    dirty = value;
}

void CacheLine::setBlockAddress(
    unsigned long long value
) {
    blockAddress = value;
}

void CacheLine::setTag(
    unsigned long long value
) {
    tag = value;
}

void CacheLine::setData(
    unsigned long long value
) {
    data = value;
}

void CacheLine::setInsertionTime(
    unsigned long long value
) {
    insertionTime = value;
}

void CacheLine::setLastUsedTime(
    unsigned long long value
) {
    lastUsedTime = value;
}