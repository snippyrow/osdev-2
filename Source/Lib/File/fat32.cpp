#include "fat32.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"

// Boot sector was already loaded, just overlay it there.
struct fat32_bootsector* fat32_info = (struct fat32_bootsector*)0x7c00;

// To convert a FAT32 cluster index to a valid LBA, formula is here:
// data_begin_lba + (cluster - 2) * sectors_per_cluster



uint32_t fat32_find_next(uint32_t current) {
    uint32_t offset = current * 4; // Since each entry in the FAT is four bytes long, find the index by multiplying by four.

    // Decode exactly which sector of the FAT we need to read.
    // Quickly allocate a space that we can use for reading the FAT sector into
    uint32_t tmp_fat[128];

    uint32_t fat_start_lba = (uint32_t)(fat32_info -> reservedSectors);
    uint32_t fat_sector_lba = fat_start_lba + (offset / fat32_info -> bytesPerSector); // Exact LBA sector that needs to be read.
    uint32_t offset_in = current % ((fat32_info -> bytesPerSector) / 4); // The exact offset within the sector.

    // Read LBA into the temporary array
    ata_lba_read(fat_sector_lba, 1, *tmp_fat);

    // Read off the 32-bit value
    uint32_t next = tmp_fat[offset_in];

    return next & 0x0FFFFFFF;
}