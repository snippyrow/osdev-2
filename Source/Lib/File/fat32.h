#ifndef FAT_H
#define FAT_H

#include "stdint.h"

#define FAT_CLUSTER_BAD 0x0FFFFFF7
// The end-of-chain value can range from ..FF8-..FFF. Anything else is the next cluster in the list.
#define FAT_EOC_START 0x0FFFFFF8
#define FAT_EOC_END 0x0FFFFFFF

// Find information from the FAT32 FS header. For more details, navigate to the first MBR file.
struct fat32_bootsector {
    uint8_t inst_jump[3]; // Jump instruction
    char oem[8]; // OEM ID
    uint16_t bytesPerSector;
    uint8_t clustersPerSector;
    uint16_t reservedSectors;
    uint8_t fat_num;
    uint8_t res[19]; // Skip useless fields
    uint32_t sectorsPerFat;
    uint8_t res2[4]; // Skip more
    uint32_t rootCluster;
} __attribute__((packed));

struct fat32_dirEntry {
    char fname[8]; // Filename (padded with spaces)
    char fext[3]; // File extension (padded with spaces)
    uint8_t attributes; // See *
    uint8_t reserved;
    uint8_t create_time_fine; // Use 10mz units.
    uint16_t create_time; // See **
    uint16_t create_date; //  See ***
    uint16_t access_date;
    uint16_t cluster_high; // High two bytes of the first cluster index in the FAT.
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low; // Low two bytes of cluster in the FAT. Also works for FAT12 & FAT16.
    uint32_t size; //Size in bytes (maximum 4GB!)
} __attribute__((packed));
// * File/folder attributes: (0x1) Read only; (0x2) Hidden; (0x4) System; (0x8) Volume label; (0x10) Subdirectory/folder; (0x20) Archive; (0x40) Device; (0x80) Reserved
// ** Creation times: (0-4) Seconds/2 (0-29); (5-10) Minutes (0-59); (11-15) Hours (0-23)
// *** Creation dates: (0-4) Day (1-31); (5-8) Month (1-12); (9-15) Year (0 = 1980)

extern struct fat32_bootsector* fat32_info;

// Read a file from the FAT into a buffer. max_read is the maximum number of bytes to try reading.
// Return 0 if success, -1 if fail
int read(uint32_t descriptor, uint8_t *buffer, uint32_t max_read);

// Create an object in a file descriptor, either a directory or file. *file is referring to a fat32_dirEntry.
int create(uint32_t descriptor, void *file);

// Remove (de-list) something from the filesystem. Descriptor is its parent directory, and the cluster helps find the exact item.
int remove(uint32_t descriptor, uint32_t cluster);

// Write to a file (Start at first byte) from the buffer. Max_write is how many bytes we should try to read, and cluster helps find the item
int write(uint32_t descriptor, uint8_t *buffer, uint32_t max_write, uint32_t cluster);

// String is exactly 11 bytes long.
int rename(uint32_t descriptor, char *str, uint32_t cluster);


#endif