
#include "CacheHierarchy.h"

#include <iostream>
#include <stdexcept>

// =========================================
// CONSTRUCTOR
// =========================================

CacheHierarchy::CacheHierarchy(
    int l1Sets,
    int l1Associativity,
    int l2Sets,
    int l2Associativity,
    ReplacementPolicy replacementPolicy,
    WritePolicy writePolicy,
    Memory& memory
)
    : l1(
          l1Sets,
          l1Associativity,
          replacementPolicy,
          writePolicy,
          memory
      ),
      l2(
          l2Sets,
          l2Associativity,
          replacementPolicy,
          writePolicy,
          memory
      ),
      memory(memory),
      writePolicy(writePolicy)
{
}

// =========================================
// INSTALL BLOCK IN L2
// =========================================

void CacheHierarchy::installL2(
    unsigned long long blockAddress,
    unsigned long long data,
    bool dirty
) {
    int index = static_cast<int>(
        blockAddress % l2.getNumberOfSets()
    );

    unsigned long long tag =
        blockAddress / l2.getNumberOfSets();

    // STEP 1: Update existing L2 block
    if (l2.lookup(tag, index)) {
        l2.fillBlockForHierarchy(
            tag,
            index,
            blockAddress,
            data,
            dirty,
            l2Stats
        );
        return;
    }

    // STEP 2: Check available space
    CacheSet& set = l2.getSet(index);
    bool full = true;

    for (int i = 0; i < set.getAssociativity(); i++) {
        if (!set.getLine(i).isValid()) {
            full = false;
            break;
        }
    }

    // STEP 3: Evict L2 victim if full
    if (full) {
        unsigned long long victimAddress = 0;
        unsigned long long victimData = 0;
        bool victimDirty = false;

        l2.evictBlock(
            index,
            victimAddress,
            victimData,
            victimDirty
        );

        // STEP 4: Check L1 for a matching block
        int l1Index = static_cast<int>(
            victimAddress % l1.getNumberOfSets()
        );

        unsigned long long l1Tag =
            victimAddress / l1.getNumberOfSets();

        unsigned long long l1Data = 0;
        bool l1Dirty = false;

        bool foundInL1 = l1.invalidateBlock(
            l1Tag,
            l1Index,
            l1Data,
            l1Dirty
        );

        if (foundInL1) {
            std::cout
                << "L1 INVALIDATED"
                << " | Block: " << victimAddress
                << "\n";

            // Prefer newer dirty data from L1
            if (l1Dirty) {
                victimData = l1Data;
                victimDirty = true;

                l1Stats.recordWriteBack();

                std::cout
                    << "DIRTY L1 DATA PRESERVED"
                    << " | Block: " << victimAddress
                    << " | Data: " << victimData
                    << "\n";
            }
        }

        // STEP 5: Write dirty data to RAM
        if (victimDirty) {
            memory.write(
                victimAddress,
                victimData
            );

            l2Stats.recordWriteBack();

            std::cout
                << "L2 WRITEBACK TO MEMORY"
                << " | Block: " << victimAddress
                << " | Data: " << victimData
                << "\n";
        }
    }

    // STEP 6: Fill L2
    l2.fillBlockForHierarchy(
        tag,
        index,
        blockAddress,
        data,
        dirty,
        l2Stats
    );
}

// =========================================
// INSTALL BLOCK IN L1
// =========================================

void CacheHierarchy::installL1(
    unsigned long long blockAddress,
    unsigned long long data,
    bool dirty
) {
    int index = static_cast<int>(
        blockAddress % l1.getNumberOfSets()
    );

    unsigned long long tag =
        blockAddress / l1.getNumberOfSets();

    // STEP 1: Update existing block
    if (l1.lookup(tag, index)) {
        l1.fillBlockForHierarchy(
            tag,
            index,
            blockAddress,
            data,
            dirty,
            l1Stats
        );
        return;
    }

    // STEP 2: Check for empty way
    CacheSet& set = l1.getSet(index);
    bool full = true;

    for (int i = 0; i < set.getAssociativity(); i++) {
        if (!set.getLine(i).isValid()) {
            full = false;
            break;
        }
    }

    // STEP 3: Evict L1 victim if necessary
    if (full) {
        unsigned long long victimAddress = 0;
        unsigned long long victimData = 0;
        bool victimDirty = false;

        l1.evictBlock(
            index,
            victimAddress,
            victimData,
            victimDirty
        );

        if (victimDirty) {
            l1Stats.recordWriteBack();

            std::cout
                << "L1 WRITEBACK TO L2"
                << " | Block: " << victimAddress
                << " | Data: " << victimData
                << "\n";

            installL2(
                victimAddress,
                victimData,
                true
            );
        }
    }

    // STEP 4: Fill L1
    l1.fillBlockForHierarchy(
        tag,
        index,
        blockAddress,
        data,
        dirty,
        l1Stats
    );
}

// =========================================
// READ OPERATION
// =========================================

void CacheHierarchy::read(
    unsigned long long address,
    int blockSize
) {
    if (blockSize <= 0) {
        throw std::invalid_argument(
            "Block size must be positive"
        );
    }

    unsigned long long blockAddress =
        address / static_cast<unsigned long long>(blockSize);

    int l1Index = static_cast<int>(
        blockAddress % l1.getNumberOfSets()
    );

    unsigned long long l1Tag =
        blockAddress / l1.getNumberOfSets();

    int l2Index = static_cast<int>(
        blockAddress % l2.getNumberOfSets()
    );

    unsigned long long l2Tag =
        blockAddress / l2.getNumberOfSets();

    unsigned long long data = 0;

    // STEP 1: Check L1
    if (l1.getData(l1Tag, l1Index, data)) {
        l1Stats.recordHit();

        std::cout
            << "L1 HIT | Data: "
            << data << "\n";

        return;
    }

    l1Stats.recordMiss();
    std::cout << "L1 MISS\n";

    // STEP 2: Check L2
    if (l2.getData(l2Tag, l2Index, data)) {
        l2Stats.recordHit();

        std::cout
            << "L2 HIT | Data: "
            << data << "\n";
    }
    else {
        l2Stats.recordMiss();

        // STEP 3: Fetch data from RAM
        data = memory.read(blockAddress);

        std::cout
            << "L2 MISS | Memory Data: "
            << data << "\n";

        installL2(
            blockAddress,
            data,
            false
        );
    }

    // STEP 4: Install in L1
    installL1(
        blockAddress,
        data,
        false
    );
}

// =========================================
// WRITE OPERATION - UPDATED
// =========================================

void CacheHierarchy::write(
    unsigned long long address,
    unsigned long long data,
    int blockSize
) {
    if (blockSize <= 0) {
        throw std::invalid_argument(
            "Block size must be positive"
        );
    }

    unsigned long long blockAddress =
        address / static_cast<unsigned long long>(blockSize);

    int l1Index = static_cast<int>(
        blockAddress % l1.getNumberOfSets()
    );

    unsigned long long l1Tag =
        blockAddress / l1.getNumberOfSets();

    int l2Index = static_cast<int>(
        blockAddress % l2.getNumberOfSets()
    );

    unsigned long long l2Tag =
        blockAddress / l2.getNumberOfSets();

    unsigned long long oldData = 0;

    // STEP 1: Check L1
    bool l1Hit = l1.getData(
        l1Tag,
        l1Index,
        oldData
    );

    if (l1Hit) {
        l1Stats.recordHit();
        std::cout << "L1 WRITE HIT\n";
    }
    else {
        l1Stats.recordMiss();
        std::cout << "L1 WRITE MISS\n";

        // STEP 2: Check L2 on write miss
        unsigned long long l2Data = 0;

        if (l2.getData(l2Tag, l2Index, l2Data)) {
            l2Stats.recordHit();
            std::cout << "L2 HIT ON WRITE MISS\n";
        }
        else {
            l2Stats.recordMiss();
            std::cout << "L2 MISS ON WRITE MISS\n";

            // Write allocate:
            // Fetch the existing block from RAM
            unsigned long long memoryData =
                memory.read(blockAddress);

            installL2(
                blockAddress,
                memoryData,
                false
            );
        }
    }

    // STEP 3: Apply write policy
    if (writePolicy == WritePolicy::WRITE_THROUGH) {

        // Immediately update RAM
        memory.write(
            blockAddress,
            data
        );

        // Update L2 with clean data
        installL2(
            blockAddress,
            data,
            false
        );

        // Update L1 with clean data
        installL1(
            blockAddress,
            data,
            false
        );
    }
    else {

        // WRITE_BACK:
        // Update L1 only and mark dirty.
        // L2 and RAM are updated during eviction.
        installL1(
            blockAddress,
            data,
            true
        );
    }

    std::cout
        << "Written Data: "
        << data << "\n";
}

// =========================================
// CACHE STATISTICS
// =========================================

const CacheStats& CacheHierarchy::getL1Stats() const {
    return l1Stats;
}

const CacheStats& CacheHierarchy::getL2Stats() const {
    return l2Stats;
}
