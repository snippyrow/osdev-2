#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "stdint.h"


int fat32_fs_read(uint32_t descriptor, uint8_t *buffer, uint32_t max_read);

uint32_t malloc(uint32_t num_blocks);

extern "C" uint32_t k_call(uint32_t sysno, uint32_t a0 = 0, uint32_t a1 = 0, uint32_t a2 = 0);

void fail();

struct fat32_dirEntry {
    char fname[11]; // Filename and extension (padded with spaces)
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

#endif