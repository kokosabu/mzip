#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "crc32.h"

static void make_crc_table(void);

static uint32_t crc_table[256];
bool is_make_crc_table = false;

static void make_crc_table(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc_table[i] = c;
    }
}

uint32_t crc32(FILE *input)
{
    uint8_t buf[1024];
    size_t size;
    uint32_t c;

    if(!is_make_crc_table) {
        make_crc_table();
    }

    fseek(input, 0, SEEK_SET);

    c = 0xFFFFFFFF;
    size = fread(buf, 1, 1024, input);
    while(size != 0) {
        for (size_t i = 0; i < size; i++) {
            c = crc_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
        }
        size = fread(buf, 1, 1024, input);
    }

    fseek(input, 0, SEEK_SET);
    return c ^ 0xFFFFFFFF;
}
