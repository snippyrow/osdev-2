#include "fat32.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"

// Boot sector was already loaded, just overlay it there.
struct fat32_bootsector* fat32_info = (struct fat32_bootsector*)0x7c00;


// Convert from a cluster to valid LBA
static inline uint32_t cluster_lba(uint32_t cluster) {
    return (fat32_info -> reservedSectors) + (fat32_info -> fat_num * fat32_info -> sectorsPerFat) + ((cluster - fat32_info -> rootCluster) * fat32_info -> clustersPerSector);
}

// Quickly allocate a space that we can use for reading the FAT sector into
uint32_t tmp_fat[128];

// In future: compare the two copies of the FAT.
uint32_t fat32_find_next(uint32_t current) {
    uint32_t offset = current * 4; // Since each entry in the FAT is four bytes long, find the index by multiplying by four.
    uint32_t firstResult;

    // Decode exactly which sector of the FAT we need to read.

    uint32_t fat_start_lba = (uint32_t)(fat32_info -> reservedSectors);
    uint32_t fat_sector_lba = fat_start_lba + (offset / fat32_info -> bytesPerSector); // Exact LBA sector that needs to be read.
    uint32_t offset_in = current % ((fat32_info -> bytesPerSector) / 4); // The exact offset within the sector.

    // Read LBA into the temporary array
    ata_lba_read(fat_sector_lba, 1, (uint32_t)tmp_fat);

    // Read off the 32-bit value
    uint32_t next = tmp_fat[offset_in];
    firstResult = next; // Handle multiple different FAT's
    if (fat32_info -> fat_num == 1) return next & 0x0FFFFFFF; // If one, end here

    for (uint16_t j = 1; j < fat32_info -> fat_num; j++) {
        fat_start_lba += (uint32_t)(fat32_info -> sectorsPerFat);
        fat_sector_lba = fat_start_lba + (offset / fat32_info -> bytesPerSector);

        // Read at the new offset
        ata_lba_read(fat_sector_lba, 1, (uint32_t)tmp_fat);
        next = tmp_fat[offset_in];
        if (next != firstResult) return FAT_CLUSTER_BAD; // Return an error, since the FAT must be corrupted.
    }

    return firstResult & 0x0FFFFFFF;
}

uint32_t fat32_write_entry(uint32_t index, uint32_t value) {
    // Update FAT at some index for all FAT entries.

    uint32_t fat_start_lba = (uint32_t)(fat32_info -> reservedSectors);
    uint32_t fat_sector_lba;

    uint32_t offset_in = index % ((fat32_info -> bytesPerSector) / 4); // The exact offset within the sector.
    for (uint16_t j = 0; j < fat32_info -> fat_num; j++) {
    
        fat_sector_lba = fat_start_lba + (index * 4 / fat32_info -> bytesPerSector); // LBA being requested
        ata_lba_read(fat_sector_lba, 1, (uint32_t)tmp_fat);

        // Modify the location and write it back into the FAT
        tmp_fat[offset_in] = value;
        ata_lba_write(fat_sector_lba, 1, (uint8_t*)tmp_fat);

        // Add to the starting LBA
        fat_start_lba += (fat32_info -> sectorsPerFat);
    }
    return 0;
}

// Iterate through the FAT in search of a new usable cluster in the FAT
// Return 0 if error
uint32_t fat32_scan_next() {
    // Start from the first LBA, go from there. If a zero cluster is found, cross-check with other FAT's.
    // This is an expensive operation, so do it in chunks of 16 for now.
    uint32_t *fatptr = kmalloc(16); // Request 32 blocks
    if (!fatptr) return 0; // Allocation failure

    uint32_t fat_start_lba = (uint32_t)(fat32_info -> reservedSectors);
    uint32_t current_cluster = 2;

    for (uint32_t sec = 0; sec < fat32_info -> sectorsPerFat; sec+=16) {
        uint32_t sectors_to_read = 16;
        if (sec + 16 > fat32_info->sectorsPerFat) { // Truncate if near the end of this FAT
            sectors_to_read = fat32_info->sectorsPerFat - sec;
        }

        if (ata_lba_read(fat_start_lba + sec, sectors_to_read, (uint32_t)fatptr) != 0) {
            kfree(fatptr, 16);
            return 0; // Teriminate if read error
        }

        uint32_t entries_this = sectors_to_read * 128;

        for (uint32_t i = 0; i < entries_this; i++) {
            uint32_t entry = fatptr[i];

            // Free cluster = 0x00000000
            if (entry == 0) {
                // Verify later that it is consistent with other copies of FAT
                // Otherwise continue

                kfree(fatptr, 16);
                return current_cluster;
            }

            current_cluster++;
        }
    }
    kfree(fatptr, 16); // Free allocated stuff
    return 0; // None found
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
        uint32_t lba = cluster_lba(cluster);
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

// Update an existing file on the filesystem. Remove unused clusters, write in new ones.
// "Descriptor" is where the file begins

// When I'm feeling more up to it:
// Read file and replace some of the data if not writing to the full sector
// Place EOC marker if needed
// Remove old clusters if needed
int write(uint32_t descriptor, uint8_t *buffer, uint32_t len) {
    // Assuming the file is already in the system and has a valid FAT entry.
    // Since there exists multiple FAT's, handle them.
    uint32_t lba_fs_begin = (fat32_info -> reservedSectors) + (fat32_info -> fat_num * fat32_info -> sectorsPerFat);
    uint32_t lba;
    uint32_t numClusters = (len / fat32_info -> bytesPerSector) + 1;
    uint32_t cluster = descriptor;

    uint8_t *fbuff = buffer; // Pointer to be manipulated
    // + ((cluster - fat32_info -> rootCluster) * fat32_info -> bytesPerSector)
    uint32_t c = len;
    for (uint32_t j = 0; j < numClusters; j++) {
        lba = ((cluster - fat32_info -> rootCluster) * fat32_info -> bytesPerSector);
        ata_lba_write(lba, 1, fbuff);
        c -= 512; // How many characters are left to write
        *fbuff += 512; // Go up the buffer

        // Find next cluster to write to
        cluster = fat32_find_next(cluster);
        if (cluster == FAT_CLUSTER_BAD) return -1; // FAT error

        // A new cluster needs to be allocated
        if (cluster >= FAT_EOC_START) {

        }
        // Otherwise simply write to the next one
    }



    // Remove unused clusters
    while (cluster >= FAT_EOC_START) {
        
    }
    return 0;
}

// Create an object inside of a subdirectory.
// Return 0 if error
uint32_t fat32_mko(uint32_t descriptor, fat32_dirEntry *object) {
    // Begin scanning the directory recursively looking for a spot
    uint32_t cluster = descriptor;
    fat32_dirEntry *subdir_ptr = (fat32_dirEntry *)kmalloc(1);
    uint32_t lba_fs_begin = (fat32_info -> reservedSectors) + (fat32_info -> fat_num * fat32_info -> sectorsPerFat);

    uint32_t prevCluster;
    uint32_t new_cluster;
    if (!subdir_ptr) return 0; // Failed to allocate
    while (cluster < FAT_EOC_START) {
        ata_lba_read(cluster_lba(cluster), 1, (uint32_t)subdir_ptr);
        prevCluster = cluster; // Save old cluster for index finding
        for (uint16_t i = 0; i < 32; i++) {
            if (subdir_ptr[i].fname[0] != 0) continue; // This spot is occupied
            // Also, you can check for the first character being 0xE5 for a deleted item.
            // Create file here
            memcpy(&subdir_ptr[i], object, sizeof(fat32_dirEntry)); // Copy into the location
            // Find a spot for the new file
            new_cluster = fat32_scan_next();
            if (!new_cluster) {
                kfree(subdir_ptr, 1);
                return 0;
            }
            fat32_write_entry(new_cluster, FAT_EOC_START); // Write it
            subdir_ptr[i].cluster_low = (uint16_t)(new_cluster & 0xffff);
            subdir_ptr[i].cluster_high = (uint16_t)((new_cluster >> 16) & 0xffff);
            ata_lba_write(cluster_lba(cluster), 1, subdir_ptr); // Write it back

            // Return location
            kfree(subdir_ptr, 1);
            return new_cluster; 
        }
        cluster = fat32_find_next(cluster);
        if (cluster == FAT_CLUSTER_BAD) { // Bad cluster
            kfree(subdir_ptr, 1);
            return 0;
        }
    }

    // If reached the end of the directory
    new_cluster = fat32_scan_next();
    if (!new_cluster) { // Something really bad happened (out of new clusters)
        kfree(subdir_ptr, 1);
        return 0;
    }

    fat32_write_entry(prevCluster, new_cluster); // Using the previous cluster as an index, link the old end-of-chain to the new cluster.
    fat32_write_entry(new_cluster, FAT_EOC_START);

    memset(subdir_ptr, 0, 512); // Clear
    memcpy(subdir_ptr, object, sizeof(fat32_dirEntry)); // Copy new struct to the start of the subdirectory


    ata_lba_write(cluster_lba(new_cluster), 1, subdir_ptr);

    kfree(subdir_ptr, 1);
    return new_cluster; 
}

// Create a subdirectory at some point, with name *name
// Return new cluster, zero if failed.
uint32_t fat32_mkdir(uint32_t descriptor, char *name) {

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
        uint32_t lba = cluster_lba(cluster);
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