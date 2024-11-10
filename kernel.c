volatile void _main();
void entry_point() {
    _main();
}

#include <stdint.h>

#define ATA_PRIMARY_BASE      0x1F0   // Primary base I/O port
#define ATA_PRIMARY_CTRL      0x3F6   // Primary control port

#define ATA_REG_DATA          0x1F0
#define ATA_REG_ERROR         0x1F1
#define ATA_REG_SECCOUNT      0x1F2
#define ATA_REG_LBA0          0x1F3
#define ATA_REG_LBA1          0x1F4
#define ATA_REG_LBA2          0x1F5
#define ATA_REG_DEVSEL        0x1F6
#define ATA_REG_STATUS        0x1F7
#define ATA_REG_COMMAND       0x1F7

#define ATA_CMD_READ_SECTORS  0x20

// Read a byte from an I/O port
uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Write a byte to an I/O port
void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Read words (16-bits) from an I/O port into a buffer
void insw(uint16_t port, uint16_t* buffer, uint32_t count) {
    __asm__ volatile ("rep insw" : "+D"(buffer), "+c"(count) : "d"(port) : "memory");
}

// IDE function to read a sector
int ide_read_sector(uint8_t channel, uint8_t drive, uint32_t lba, uint16_t* buffer) {
    // Check if the channel is valid (0 for primary, 1 for secondary)
    if (channel > 1) {
        return -1; // Invalid channel (only primary or secondary allowed)
    }

    // Select the base I/O port for the specified channel
    uint16_t base = (channel == 0) ? ATA_PRIMARY_BASE : 0x170; // 0x170 is the secondary base address

    // 1. Select drive (0xE0 is for the master drive, 0xF0 is for the slave drive)
    outb(base + ATA_REG_DEVSEL, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));

    // 2. Write the sector count (1 sector)
    outb(base + ATA_REG_SECCOUNT, 1);

    // 3. Write the LBA address (for 28-bit LBA)
    outb(base + ATA_REG_LBA0, (uint8_t) lba);
    outb(base + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(base + ATA_REG_LBA2, (uint8_t)(lba >> 16));

    // 4. Send the read command
    outb(base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    // 5. Polling: Wait for BSY to clear and DRQ to set
    while (inb(base + ATA_REG_STATUS) & 0x80);  // Wait for BSY to clear
    uint8_t status = inb(base + ATA_REG_STATUS);

    // Check for errors
    if (status & 0x01) {
        return -1; // Error flag set
    }

    if (!(status & 0x08)) {
        return -2; // DRQ flag not set (data not ready)
    }

    // 6. Read data (256 words = 512 bytes)
    insw(base + ATA_REG_DATA, buffer, 256);

    return 0; // Return 0 on success
}

volatile void _main() {
    uint16_t buffer[256];  // Buffer to hold sector data (512 bytes)
    ide_read_sector(0, 0, 64, buffer);  // Read sector 64 from the primary master drive

    volatile char *video_memory = (volatile char *)0xb8000;

    // Display the buffer contents in video memory
    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i];

        // Extract the nibbles from the 16-bit data
        int high_nibble_1 = (data >> 12) & 0x0F;  // First nibble (most significant)
        int low_nibble_1 = (data >> 8) & 0x0F;   // Second nibble

        int high_nibble_2 = (data >> 4) & 0x0F;  // Third nibble
        int low_nibble_2 = data & 0x0F;          // Fourth nibble

        // Convert each nibble to ASCII hex characters (0-9, A-F)
        video_memory[i * 8] = (high_nibble_1 > 9 ? (high_nibble_1 - 10 + 'A') : (high_nibble_1 + '0'));
        video_memory[i * 8 + 1] = 0x0F;  // Light grey on black (VGA attribute byte)

        video_memory[i * 8 + 2] = (low_nibble_1 > 9 ? (low_nibble_1 - 10 + 'A') : (low_nibble_1 + '0'));
        video_memory[i * 8 + 3] = 0x0F;  // Light grey on black (VGA attribute byte)

        video_memory[i * 8 + 4] = (high_nibble_2 > 9 ? (high_nibble_2 - 10 + 'A') : (high_nibble_2 + '0'));
        video_memory[i * 8 + 5] = 0x0F;  // Light grey on black (VGA attribute byte)

        video_memory[i * 8 + 6] = (low_nibble_2 > 9 ? (low_nibble_2 - 10 + 'A') : (low_nibble_2 + '0'));
        video_memory[i * 8 + 7] = 0x0F;  // Light grey on black (VGA attribute byte)
    }
}
