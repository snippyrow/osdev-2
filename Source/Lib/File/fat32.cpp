#include "fat32.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"

// Boot sector was already loaded, just overlay it there.
struct fat32_bootsector* fat32_info = (struct fat32_bootsector*)0x7c00;

// To convert a FAT32 cluster index to a valid LBA, formula is here:
// data_begin_lba + (cluster - 2) * sectors_per_cluster



// In future: compare the two copies of the FAT.
uint32_t fat32_find_next(uint32_t current) {
    uint32_t offset = current * 4; // Since each entry in the FAT is four bytes long, find the index by multiplying by four.

    // Decode exactly which sector of the FAT we need to read.
    // Quickly allocate a space that we can use for reading the FAT sector into
    uint32_t tmp_fat[128];

    uint32_t fat_start_lba = (uint32_t)(fat32_info -> reservedSectors);
    uint32_t fat_sector_lba = fat_start_lba + (offset / fat32_info -> bytesPerSector); // Exact LBA sector that needs to be read.
    uint32_t offset_in = current % ((fat32_info -> bytesPerSector) / 4); // The exact offset within the sector.

    // Read LBA into the temporary array
    ata_lba_read(fat_sector_lba, 1, (uint32_t)tmp_fat);

    // Read off the 32-bit value
    uint32_t next = tmp_fat[offset_in];
    return next & 0x0FFFFFFF;
}

int read(uint32_t descriptor, uint8_t *buffer, uint32_t max_read) {
    uint8_t tmp_chunk[512]; // Temporary allocated block for reading in parts of the file
    uint32_t read = 0; // Total number of bytes read so far

    uint32_t cluster = descriptor; // Where to begin reading
    while (cluster < FAT_EOC_START && read < max_read) {
        if (cluster == FAT_CLUSTER_BAD) {
            return -1; // Bad cluster, terminate
        }
        // Find the LBA needed to read the file from after the FAT
        uint32_t lba = (fat32_info -> reservedSectors) + (fat32_info -> fat_num * fat32_info -> sectorsPerFat) + ((cluster - fat32_info -> rootCluster) * fat32_info -> clustersPerSector);
        int err = ata_lba_read(lba, 1, (uint32_t)tmp_chunk);
        if (err) { // If the disk driver gives an error, terminate the read.
            return -1;
        }

        for (uint16_t i = 0; i < 512; i++) {
            buffer[read] = tmp_chunk[i];
            read++;
            if (read >= max_read) {
                return 0; // End if we read all the bytes required by max_read
            }
        }

        // If there is more data, read the next cluster
        cluster = fat32_find_next(cluster);
    }

    return 0;
}

// Assume that only one file has the specific name.
uint32_t find(uint32_t descriptor, char *str) {
    // Each physical disk sector has 16 directory entries. Read it like a file, search each one until a file is found.
    fat32_dirEntry tmp_chunk[16]; // Read clusters of the directory here.
    fat32_dirEntry* ptr = tmp_chunk;

    uint32_t cluster = descriptor; // Where to begin reading
    while (cluster < FAT_EOC_START) {
        if (cluster == FAT_CLUSTER_BAD) {
            return 0; // Bad cluster, terminate
        }

        // Find the LBA needed to read the file from after the FAT
        uint32_t lba = (fat32_info -> reservedSectors) + (fat32_info -> fat_num * fat32_info -> sectorsPerFat) + ((cluster - fat32_info -> rootCluster) * fat32_info -> bytesPerSector);
        int err = ata_lba_read(lba, 1, (uint32_t)ptr);
        if (err) { // If the disk driver gives an error, terminate the read.
            return 0;
        }

        for (uint8_t i = 0; i < 16; i++) {
            // Check if end of directory
            if (tmp_chunk[i].fname[0] == 0) return 0; // If end of directroy, terminate.
            // First check if the thing is a file. Then check if the strings match.
            uint8_t strmatch = 0; // If both strings match all the way through, strmatch should be equal to 0
            for (uint8_t j = 0; j < 11; j++) {
                strmatch |= (tmp_chunk[i].fname[j] ^ str[j]);
            }
            if (!strmatch && !(tmp_chunk[i].attributes & 0x10)) { // Do strings match, and is it NOT a directory?
                return (tmp_chunk[i].cluster_high << 16) + tmp_chunk[i].cluster_low;
            }
        }

        // If there is more data, read the next cluster
        cluster = fat32_find_next(cluster);
    }

    return 0; // No file found
}