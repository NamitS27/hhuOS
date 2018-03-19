#include "StorageDevice.h"

StorageDevice::StorageDevice(const String &name) : name(name) {

}

String StorageDevice::getName() {
    return name;
}

Util::ArrayList<StorageDevice::PartitionInfo>& StorageDevice::readPartitionTable() {
    partLock.lock();

    partitionList.clear();

    uint8_t mbr[getSectorSize()];
    if(!read(mbr, 0, 1)) {
        partLock.unlock();
        return partitionList;
    }

    // Check MBR-Signature
    uint16_t signature = *((uint16_t *)(&mbr[510]));
    if(signature != 0xaa55) {
        partLock.unlock();
        return partitionList;
    }

    auto *partitions = (PartitionTableEntry *) &mbr[PARTITON_TABLE_START];
    
    // Cycle through all four primary partitions
    for(uint8_t i = 0; i < 4; i++) {
        PartitionTableEntry currentPartition = partitions[i];

        // Check system ID
        // 0 --> Unused partiton
        if(currentPartition.system_id == EMPTY)
            continue;

        // 5 --> Extended partiton
        if(currentPartition.system_id == EXTENDED_PARTITION || currentPartition.system_id == EXTENDED_PARTITION_LBA) {
            auto *currentPart = new StorageDevice::PartitionInfo{'e', currentPartition.active_flag, currentPartition.system_id, currentPartition.relative_sector, currentPartition.sector_count};
            partitionList.add(*currentPart);
            uint32_t nextLogicalMbr = currentPartition.relative_sector;

            // An extended partition contains a linked list of logical partitions:
            // The first partition table entry of an extended partition's MBR contains contains information about the currrent logical partition.
            // The second partition table entry points to the next logical MBR which works exactly the same.
            while(true) {
                uint8_t logicalMbr[getSectorSize()];
                read(logicalMbr, nextLogicalMbr, 1);

                // Check MBR-Signature
                signature = *((uint16_t *)(&logicalMbr[510]));
                if(signature != 0xaa55)
                    break;

                PartitionTableEntry currentLogicalPartition = *((PartitionTableEntry *) &logicalMbr[PARTITON_TABLE_START]);
                PartitionTableEntry nextLogicalPartition = *((PartitionTableEntry *) &logicalMbr[PARTITON_TABLE_START + 0x10]);

                // Check system ID
                // 0 --> Unused partiton
                if(currentLogicalPartition.system_id == EMPTY)
                    break;

                currentPart = new StorageDevice::PartitionInfo{'l', currentLogicalPartition.active_flag, currentLogicalPartition.system_id, nextLogicalMbr + currentLogicalPartition.relative_sector, currentLogicalPartition.sector_count};
                partitionList.add(*currentPart);

                // Check system ID
                // 0 --> Unused partiton
                if(nextLogicalPartition.system_id == EMPTY)
                    break;
                
                // Calculate address of the next MBR
                nextLogicalMbr = currentPartition.relative_sector + nextLogicalPartition.relative_sector;
            }
        } else {
            auto *currentPart = new StorageDevice::PartitionInfo{'p', currentPartition.active_flag, currentPartition.system_id, currentPartition.relative_sector, currentPartition.sector_count};
            partitionList.add(*currentPart);
        }
    }

    partLock.unlock();
    return partitionList;
}

uint32_t StorageDevice::writePartition(uint8_t partNumber, bool active, uint8_t systemId, uint32_t startSector, uint32_t sectorCount) {
    partLock.lock();

    uint8_t mbr[getSectorSize()];
    if(!read(mbr, 0, 1)) {
        partLock.unlock();
        return READ_SECTOR_FAILED;
    }

    // Check MBR-Signature
    uint16_t signature = *((uint16_t *)(&mbr[510]));
    if(signature != 0xaa55) {
        partLock.unlock();
        return INVALID_MBR_SIGNATURE;
    }

    PartitionTableEntry partEntry{
        (uint8_t)(active ? 80 : 0), // 0x80 --> bootable partition, 0x0 --> non-bootable partition
        0,                          // Starting head of the partition
        0,                          // Bits 0-6: Starting sector of the partition, Bits 7-15: Starting cylinder of the partition
        systemId,                   // Partition type identifier
        0,                          // Ending head of the partition
        0,                          // Bits 0-6: Ending sector of the partition, Bits 7-15: Ending cylinder of the partition
        startSector,                // Relative sector to start of partition
        sectorCount,                // Amount of sectors in partition
    };

    if(partNumber < 4) {
        // Primary partition
        auto *partPtr = (PartitionTableEntry *) &mbr[PARTITON_TABLE_START + 0x10 * (partNumber - 1)];

        if(systemId == 0x05 || systemId == 0x0f)  {
            // Initialize first logical mbr
            uint8_t logicalMbr[getSectorSize()];
            memset(logicalMbr, 0, getSectorSize());

            // Write mbr-signature
            *((uint16_t *)(&logicalMbr[510])) = 0xaa55;

            // Write first logical mbr
            if(!write(logicalMbr, startSector, 1)) {
                partLock.unlock();
                return WRITE_SECTOR_FAILED;
            }
        }

        // Write partition entry
        *partPtr = partEntry;
        if(!write(mbr, 0, 1)) {
            partLock.unlock();
            return WRITE_SECTOR_FAILED;
        }

        return SUCCESS;
    } else {
        // Logical partition
        PartitionTableEntry extPart{};
        uint8_t extPartIndex;
        
        // Search for extended partition
        auto *partitions = (PartitionTableEntry *) &mbr[PARTITON_TABLE_START];
        for(extPartIndex = 0; extPartIndex < 4; extPartIndex++) {
            PartitionTableEntry currentPartition = partitions[extPartIndex];

            if(currentPartition.system_id == EXTENDED_PARTITION || currentPartition.system_id == EXTENDED_PARTITION_LBA) {
                extPart = currentPartition;
                break;
            }

            if(extPartIndex == 3) {
                partLock.unlock();
                return EXTENDED_PARTITION_NOT_FOUND;
            }
        }
        
        // Iterate through logical partitions to the desired partition number or until the end of the list
        uint8_t i;
        uint8_t currentMbr[getSectorSize()];
        uint32_t currentLogicalMbr = extPart.relative_sector;
        bool appendToList = false;

        for(i = 5; i <= partNumber; i++) {
            if(!read(currentMbr, currentLogicalMbr, 1)) {
                partLock.unlock();
                return READ_SECTOR_FAILED;
            }

            // Check MBR-Signature
            signature = *((uint16_t *)(&currentMbr[510]));
            if(signature != 0xaa55) {
                partLock.unlock();
                return INVALID_MBR_SIGNATURE;
            }

            PartitionTableEntry currentLogicalPartition = *((PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START]);
            PartitionTableEntry nextLogicalPartition = *((PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START + 0x10]);
            
            if(currentLogicalPartition.system_id == EMPTY || i == partNumber)
                break;
            
            if(nextLogicalPartition.system_id == EMPTY) {
                appendToList = true;
                break;
            }

            // Calculate address of the next MBR
            currentLogicalMbr = extPart.relative_sector + nextLogicalPartition.relative_sector;
        }

        if(appendToList) {
            // Append a new partition to the list
            partEntry.relative_sector = startSector - extPart.relative_sector;

            auto *partPtr = (PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START + 0x10];
            *partPtr = partEntry;
            partPtr->system_id = EXTENDED_PARTITION_LBA;
            if(!write(currentMbr, currentLogicalMbr, 1)) {
                partLock.unlock();
                return WRITE_SECTOR_FAILED;
            }
            
            uint8_t newLogicalMbr[getSectorSize()];
            memset(newLogicalMbr, 0, getSectorSize());
            partEntry.relative_sector = 1;
            partEntry.sector_count -= 1;

            partPtr = (PartitionTableEntry *) &newLogicalMbr[PARTITON_TABLE_START];
            *partPtr = partEntry;
            *((uint16_t *)(&newLogicalMbr[510])) = 0xaa55;

            if(!write(newLogicalMbr, startSector, 1)) {
                partLock.unlock();
                return WRITE_SECTOR_FAILED;
            }

            return SUCCESS;
        } else {
            // Edit an existing partition
            auto *partPtr = (PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START];
            partPtr->relative_sector = partPtr->relative_sector == 0 ? 1 : partPtr->relative_sector;
            partPtr->sector_count = sectorCount;
            partPtr->system_id = systemId;
            partPtr->active_flag = static_cast<uint8_t>(active ? 0x80 : 0x0);

            // Write partition entry
            if(!write(currentMbr, currentLogicalMbr, 1)) {
                partLock.unlock();
                return WRITE_SECTOR_FAILED;
            }

            return SUCCESS;
        }
    }
}

uint32_t StorageDevice::deletePartition(uint8_t partNumber) {
    partLock.lock();

    uint8_t mbr[getSectorSize()];
    if(!read(mbr, 0, 1)) {
        partLock.unlock();
        return WRITE_SECTOR_FAILED;
    }

    PartitionTableEntry partEntry{0, 0, 0, 0, 0, 0, 0, 0};

    if(partNumber <= 4) {
        // Primary parition
        auto *partPtr = (PartitionTableEntry *) &mbr[PARTITON_TABLE_START + 0x10 * (partNumber - 1)];
        
        // Write partition table
        *partPtr = partEntry;
        if(!write(mbr, 0, 1)) {
            partLock.unlock();
            return WRITE_SECTOR_FAILED;
        }

        partLock.unlock();
        return SUCCESS;
    }
    
    // Logical partition
    PartitionTableEntry extPart{};
    uint8_t extPartIndex;
    
    // Search for extended partition
    auto *partitions = (PartitionTableEntry *) &mbr[PARTITON_TABLE_START];
    for(extPartIndex = 0; extPartIndex < 4; extPartIndex++) {
        PartitionTableEntry currentPartition = partitions[extPartIndex];

        if(currentPartition.system_id == EXTENDED_PARTITION || currentPartition.system_id == EXTENDED_PARTITION_LBA) {
            extPart = currentPartition;
            break;
        }

        if(extPartIndex == 3) {
            partLock.unlock();
            return EXTENDED_PARTITION_NOT_FOUND;
        }
    }

    if(partNumber == 5) {
        // Special case for first logical partition
        uint8_t firstLogicalMbr[getSectorSize()];
        if(!read(firstLogicalMbr, extPart.relative_sector, 1)) {
            partLock.unlock();
            return READ_SECTOR_FAILED;
        }
        
        PartitionTableEntry *firstMbrFirstLogicalPartition = ((PartitionTableEntry *) &firstLogicalMbr[PARTITON_TABLE_START]);
        PartitionTableEntry *firstMbrSecondLogicalPartition = ((PartitionTableEntry *) &firstLogicalMbr[PARTITON_TABLE_START + 0x10]);
        
        if(firstMbrSecondLogicalPartition->system_id == EMPTY) {
            // The first logical partition is the only one
            memset(firstMbrFirstLogicalPartition, 0, sizeof(PartitionTableEntry));
        } else {
            // There is more than one logical partition
            // We need to read the second one and let the linked list start with it
            uint8_t secondLogicalMbr[getSectorSize()];
            if(!read(secondLogicalMbr, extPart.relative_sector + firstMbrSecondLogicalPartition->relative_sector, 1)) {
                partLock.unlock();
                return READ_SECTOR_FAILED;
            }

            PartitionTableEntry *secondMbrFirstLogicalPartition = ((PartitionTableEntry *) &secondLogicalMbr[PARTITON_TABLE_START]);
            PartitionTableEntry *secondMbrSecondLogicalPartition = ((PartitionTableEntry *) &secondLogicalMbr[PARTITON_TABLE_START + 0x10]);

            firstMbrFirstLogicalPartition->relative_sector = firstMbrSecondLogicalPartition->relative_sector + secondMbrFirstLogicalPartition->relative_sector;
            firstMbrFirstLogicalPartition->sector_count = secondMbrFirstLogicalPartition->sector_count;
            *firstMbrSecondLogicalPartition = *secondMbrSecondLogicalPartition;
        }

        if(!write(firstLogicalMbr, extPart.relative_sector, 1)) {
            partLock.unlock();
            return WRITE_SECTOR_FAILED;
        }

        partLock.unlock();
        return SUCCESS;
    }

    // Iterate through logical partitions to the desired partition number or until the end of the list
    uint8_t currentMbr[getSectorSize()];
    uint32_t lastLogicalMbr = 0;
    uint32_t currentLogicalMbr = extPart.relative_sector;
    PartitionTableEntry currentLogicalPartition{};
    PartitionTableEntry nextLogicalPartition{};

    for(uint8_t i = 5; i <= partNumber; i++) {
        if(!read(currentMbr, currentLogicalMbr, 1)) {
            partLock.unlock();
            return READ_SECTOR_FAILED;
        }

        // Check MBR-Signature
        uint16_t signature = *((uint16_t *)(&currentMbr[510]));
        if(signature != 0xaa55) {
            partLock.unlock();
            return INVALID_MBR_SIGNATURE;
        }

        currentLogicalPartition = *((PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START]);
        nextLogicalPartition = *((PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START + 0x10]);

        // Check system ID
        // 0 --> Unused partition
        if(currentLogicalPartition.system_id == EMPTY) {
            partLock.unlock();
            return UNUSED_PARTITION;
        }
        
        if(nextLogicalPartition.system_id == EMPTY || i == partNumber) {
            if(i < partNumber) {
                partLock.unlock();
                return NON_EXISTENT_PARITION;
            }

            break;
        }

        // Calculate address of the next MBR
        lastLogicalMbr = currentLogicalMbr;
        currentLogicalMbr = extPart.relative_sector + nextLogicalPartition.relative_sector;
    }

    uint8_t lastMbr[getSectorSize()];
    if(!read(lastMbr, lastLogicalMbr, 1)) {
        partLock.unlock();
        return READ_SECTOR_FAILED;
    }

    // Delete partition
    auto *partPtr = (PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START];
    *partPtr = partEntry;
    partPtr = (PartitionTableEntry *) &currentMbr[PARTITON_TABLE_START + 0x10];
    *partPtr = partEntry;

    // Let predecessor point to successor
    partPtr = (PartitionTableEntry *) &lastMbr[PARTITON_TABLE_START + 0x10];
    *partPtr = nextLogicalPartition;
    
    if(!write(currentMbr, currentLogicalMbr, 1) || !write(lastMbr, lastLogicalMbr, 1)) {
        partLock.unlock();
        return WRITE_SECTOR_FAILED;
    }

    partLock.unlock();
    return SUCCESS;
}


uint32_t StorageDevice::createPartitionTable() {
    partLock.lock();

    uint8_t mbr[getSectorSize()];

    // Create new empty MBR
    memset(mbr, 0, 510);

    // Write MBR-signature
    *((uint16_t *)(&mbr[510])) = 0xaa55;

    if(!write(mbr, 0, 1)) {
        partLock.unlock();
        return WRITE_SECTOR_FAILED;
    }

    partLock.unlock();
    return SUCCESS;
}

uint8_t StorageDevice::getSystemId() {
    return 0;
}