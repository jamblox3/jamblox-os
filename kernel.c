volatile char *video_memory = (volatile char *)0xb8000;
volatile int cursor_position = 0;
volatile void _main();
void entry_point()
{
    _main();
}

//define constants
#define primary 0x1f0
#define secondary 0x170
#define reg_data 0
#define reg_error 1
#define reg_sector_count 2
#define reg_lba_low 3
#define reg_lba_mid 4
#define reg_lba_high 5
#define reg_drive 6
#define reg_status 7
#define reg_cmd 7

//define functions
char nibble_to_ascii(char input){
    if (input < 10){
        return(input+'0');
    }
    else{
        return(input-10+'A');
    }
}

void debug_print(char* string){
    for (int i=0; string[i] != 0; i++){
        if (string[i] == '\n'){
            int current_row = cursor_position / (80 * 2);
            cursor_position = (current_row + 1) * 80 * 2;
        }
        else{
            video_memory[cursor_position++] = string[i];
            video_memory[cursor_position++] =  0x0f;
        }
    }
}

void print_hex(int data) {
    char hex_str[9];
    for (int i = 0; i < 8; i++) {
        hex_str[i] = nibble_to_ascii((data >> (28 - i * 4)) & 0xF);
    }
    hex_str[8] = '\0';
    debug_print(hex_str);  // Use your existing debug_print function here
}

char inb(short port) {
    char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void outb(short port, char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

short inw(short port) {
    short value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

short outw(short port, short value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

void wait_for_ready(int base){
    int timout=100000;
    while((inb(base + reg_status) > 0x7f) && timout > 0){timout--;}
}

void handle_errors(int base){
    char error=inb(base+reg_error);
    if(error > 0){
        debug_print("disk error register is 0x");
        video_memory[cursor_position++] = nibble_to_ascii((error >> 4) & 0x0f);
        video_memory[cursor_position++] = 0x0f;
        video_memory[cursor_position++] = nibble_to_ascii(error & 0x0f);
        video_memory[cursor_position++] = 0x0f;
        char status = inb(base+reg_status);
        debug_print(" and status register is 0x");
        video_memory[cursor_position++] = nibble_to_ascii((status >> 4) & 0x0f);
        video_memory[cursor_position++] = 0x0f;
        video_memory[cursor_position++] = nibble_to_ascii(status & 0x0f);
        video_memory[cursor_position++] = 0x0f;
        debug_print("\n");
    }
    else{
        char status = inb(base+reg_status);
        debug_print("status register is 0x");
        video_memory[cursor_position++] = nibble_to_ascii((status >> 4) & 0x0f);
        video_memory[cursor_position++] = 0x0f;
        video_memory[cursor_position++] = nibble_to_ascii(status & 0x0f);
        video_memory[cursor_position++] = 0x0f;
        debug_print("\n");
    }
}

void read_ata_sectors(int sector_number, int sector_count, char drive_number, char channel, short* buffer){
    int base;
    if(channel){
        base = primary;
    }
    else{
        base = secondary;
    }
    wait_for_ready(base);
    outb(base+reg_drive,(0xE0 | (drive_number << 4) | ((sector_number >> 24) & 0x0f)));
    outb(base+reg_sector_count,sector_count);
    outb(base+reg_lba_low,sector_number & 0xff);
    outb(base+reg_lba_mid,(sector_number >> 8) & 0xff);
    outb(base+reg_lba_high,(sector_number >> 16) & 0xff);
    outb(base+reg_cmd,0x20);
    wait_for_ready(base);
    handle_errors(base);
        // Read 512 bytes from the data port (typically 256 16-bit words)
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(base); // Read 16 bits (2 bytes) per iteration
    }
}

void read_atapi_sectors(int sector_number, int sector_count, char drive_number, char channel, short* buffer){
    int base;
    if(channel){
        base = primary;
    }
    else{
        base = secondary;
    }
    wait_for_ready(base);
    outb(base + reg_drive,(0xE0 | ((drive_number << 4) & 0x0f)));
    outb(base + reg_cmd, 0xA0);

    char packet[12] = {0};
    packet[0] = 0x28;  // Read (10) command
    packet[2] = (sector_number >> 24) & 0xFF;
    packet[3] = (sector_number >> 16) & 0xFF;
    packet[4] = (sector_number >> 8) & 0xFF;
    packet[5] = sector_number & 0xFF;
    packet[7] = (sector_count >> 8) & 0xFF;
    packet[8] = sector_count & 0xFF;

    // Write the packet
    for (int i = 0; i < 6; i++) {
        outw(base + reg_data, ((short*)packet)[i]);
    }

    wait_for_ready(base);

    // Read data
    for (int i = 0; i < 256 * sector_count; i++) {
        buffer[i] = inw(base);
    }
}

void convert_short_to_int(short* short_buffer, int* int_buffer, int short_len) {
    int int_len = short_len / 2;  // Since two shorts make up one int
    for (int i = 0; i < int_len; i++) {
        // Combine two shorts into one int
        int_buffer[i] = (short_buffer[i*2] << 16) | (short_buffer[i*2 + 1] & 0xFFFF);
    }
}


volatile void _main() {
    short buffer[256];
    int int_buffer[128];
    read_atapi_sectors(64,1,0,0,buffer);
    convert_short_to_int(buffer, int_buffer, 256);
    for (int i=0; i<128; i++){
        print_hex(int_buffer[i]);
    }
}