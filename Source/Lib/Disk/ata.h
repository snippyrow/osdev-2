#ifndef ATA
#define ATA

#include "stdint.h"
#include "stdio.h"

// Define ATA-Specific I/O ports.
#define ATA_CH0_BASE 0x1f0
#define ATA_CH0_CTRL 0x3f6
#define ATA_CH1_BASE 0x170
#define ATA_CH1_CTRL 0x376

// Other ATA register offsets
#define ATA_REG_ERROR 0x01    // Error register
#define ATA_REG_SECTOR_COUNT 0x02  // Sector count register
#define ATA_REG_LBA_LOW 0x03    // LBA low byte
#define ATA_REG_LBA_MID 0x04    // LBA mid byte
#define ATA_REG_LBA_HIGH 0x05   // LBA high byte
#define ATA_REG_DRIVE_SELECT 0x06  // Drive select
#define ATA_REG_COMMAND 0x07    // Command register

// Status flags
#define ATA_SR_BSY 0x80    // Busy
#define ATA_SR_DRDY 0x40   // Drive ready
#define ATA_SR_DRQ 0x08    // Data request ready
#define ATA_SR_ERR 0x1

// Structure to figure out whats going on with the drives.
typedef struct ata_dev {
    bool exists; // Does it exist?
    uint8_t channel; // Described in the ata channel list
    uint8_t drive; // Master or slave drive?
    uint32_t size; // Number of LBA's on the drive
    char model[40]; // Allocate 40 bytes for a manufacturer model
} ata_dev;

// Describes one ATA channel (easier to write this way)
typedef struct {
    uint16_t base;
    uint16_t ctrl;
} ata_ch_desc;

extern ata_dev ata_devicelist[4];


// Read from first floppy (device that we're booting from) or C:/ drive.
// LBA Start is what LBA the read is starting from. Sector count is how many sectors we want to read, and buffer is where the read will be stored.
// Each sector is 512 bytes on the drive, and kmalloc also works in blocks of 512.
// Return value is either 1 or 0 for a success or failure.
int ata_lba_read(uint32_t lba_start, uint32_t sector_cnt, uint32_t buffer);

// LBA write does entire blocks, it does not write only parts of blocks. Buffers to be written must be in increments of 512.
int ata_lba_write(uint32_t lba_start, uint32_t sector_cnt, uint32_t buffer);

void ata_init();

#endif