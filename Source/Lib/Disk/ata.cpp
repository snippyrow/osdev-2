#include "ata.h"
#include "stdio.h"

// https://forum.osdev.org/viewtopic.php?t=56753
// "heavily inspired"

struct ata_dev ata_devicelist[4];

ata_ch_desc ata_channels[2] = {
    {.base = 0x1f0, .ctrl = 0x3f6},
    {.base = 0x170, .ctrl = 0x376}
};

// Wait for data to be ready
void ata_io_wait() {
    while (inb(ata_channels[0].base + ATA_REG_COMMAND) & ATA_SR_BSY);
    return;
}

void ata_io_wait_longer() {
    for (uint8_t x = 0; x < 12; x++) {
        inb(ata_channels[0].base + 0x7); // Poll the ATA "alternate status" port four times to make a delay.
    }
}

int ata_lba_read(uint32_t lba_start, uint32_t sector_cnt, uint32_t buffer) {
    uint16_t base = ata_channels[0].base; // Primary channel base IO

    if (sector_cnt == 0 || sector_cnt > 0xFF) return -1; // Too many sectors at once for LBA28

    //ata_io_wait(); // Wait for drive to be ready

    // Drive select + highest 4 bits of LBA
    outb(base + ATA_REG_DRIVE_SELECT, 0xE0 | ((lba_start >> 24) & 0x0F));

    ata_io_wait();

    // Set up how many sectors we need, where the LBA address is, and then send the command (0x20) to READ.
    outb(base + ATA_REG_SECTOR_COUNT, (uint8_t)sector_cnt);
    outb(base + ATA_REG_LBA_LOW, (uint8_t)(lba_start >> 0));
    outb(base + ATA_REG_LBA_MID, (uint8_t)(lba_start >> 8));
    outb(base + ATA_REG_LBA_HIGH, (uint8_t)(lba_start >> 16));

    outb(base + ATA_REG_COMMAND, 0x20); // READ SECTORS

    // Wait for drive to acknowledge command
    ata_io_wait();

    // Now read sector_count × 256 words (512 bytes)
    uint16_t* ptr = (uint16_t*)buffer;
    for (uint32_t i = 0; i < sector_cnt; i++) {
        // Wait for DRQ to be ready before trying to read each sector.
        while (!(inb(base + ATA_REG_COMMAND) & ATA_SR_DRQ));
        ata_io_wait();

        // Check for errors before reading
        uint8_t status = inb(base + ATA_REG_COMMAND);
        uint8_t err = (status & 0x01) ? inb(base + ATA_REG_ERROR) : 0;
        if (err & 0x04) return -1; // Driver bug occured (?)
        if (err & 0x10) return -1; // Invalid LBA to be read, terminate request.


        // Read 256 words (one sector)
        for (uint16_t j = 0; j < 256; j++) {
            *ptr++ = inw(base);
        }
    }

    ata_io_wait();

    // Check final status
    uint8_t final_status = inb(base + ATA_REG_COMMAND);
    if (final_status & ATA_SR_ERR) return -1;
    

    return 0;
}

void ata_init() {
    // Define channels for ATA devices
    // Four devices, a master and slave device will share the same I/O base.
    uint8_t id = 0;
    for (uint8_t ch = 0; ch < 2; ch++) {
        for (uint8_t dr = 0; dr < 2; dr++) {
            ata_devicelist[id].channel = ch;
            ata_devicelist[id].exists = false;
            ata_devicelist[id].drive = dr;
            id++;
        }
    }
    // I would finish this when more things are done.
    return;
    // Enumerate each drive to spec it out.
    for (uint8_t id = 0; id < 4; id++) {
        uint8_t channel = ata_devicelist[id].channel;
        uint8_t drive = ata_devicelist[id].drive;
        // https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
        // This command will select, either from the first or second IO base, the master or slave drive.
        // Neat nomenclature.
        outb(ata_channels[channel].base + 6, (uint8_t []){0xA0, 0xB0}[drive]);
        ata_io_wait();
        
        // Set LBA's to 0
        outb(ata_channels[channel].base + ATA_REG_LBA_LOW, 0);
        ata_io_wait();
        outb(ata_channels[channel].base + ATA_REG_LBA_MID, 0);
        ata_io_wait();
        outb(ata_channels[channel].base + ATA_REG_LBA_HIGH, 0);
        ata_io_wait();

        outb(ata_channels[channel].base + ATA_REG_COMMAND, 0xec); // Issue the IDENTIFY command.
        ata_io_wait_longer();

        if (inb(ata_channels[channel].base + ATA_REG_COMMAND) == 0) {
            continue; // Skip if drive does not exist.
        }

        // Begin polling the drive for more useful information

        uint8_t error = 0, status = 0;

        
        while((status & ATA_SR_BSY) || !(status & ATA_SR_DRQ)) {
            status = inb(ata_channels[channel].base + ATA_REG_COMMAND);
            if(status & ATA_SR_ERR){ // If Err, Device is not ATA.
                error = inb(ata_channels[channel].base + ATA_REG_ERROR);
                continue;
                // fuck you, figure out errors later.
                //break;
            }
        }

        // Read device data
        uint8_t buffer[512];
        insw(ata_channels[channel].base, (uint16_t *)buffer, 256);
        
        ata_devicelist[id].exists = true;
        // leaving here, go through the forum post and figure out how to add things like size, type, model name.
    }

}