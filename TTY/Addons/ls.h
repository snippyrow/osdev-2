#ifndef LS_H
#define LS_H

#include "stdint.h"

/*
List all files in the root/given dierctory of the shell.
Also provides volume information for the FAT32 filesystem. Basically, this command is a fat32 debugger,
*/

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

// fat32 directory entry is already defined.

int c_list_files();

#endif