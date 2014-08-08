#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "zip.h"

static uint32_t crc_table[256];
static uint16_t total = 0;
static uint32_t size = 0;
static fpos_t fpos = 0;
static bool fpos_b = false;

static void initZipHeader(FILE *output, char *file_name, FILE *input);
static void fileCopy(FILE *output, FILE *input);
static uint32_t crc32(FILE *input);
static long getFileSize(FILE *file);

/* 事前にこの関数を実行しておくこと */
void make_crc_table(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc_table[i] = c;
    }
}

static uint32_t crc32(FILE *input)
{
    uint8_t buf[1024];
    size_t size;
    uint32_t c;

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

void addZip(FILE *output, char *file_name)
{
    FILE *input;
    
    input = fopen(file_name, "rb");
    initZipHeader(output, file_name, input);
    fileCopy(output, input);
    fclose(input);
}

void addCentralDirectoryHeader(FILE *output, char *file_name)
{
    FILE *input;
    central_directory_file_header *header;

    input = fopen(file_name, "rb");
    header = (central_directory_file_header *)malloc(sizeof(central_directory_file_header) + strlen(file_name) - 1);

    header->signature                = 0x02014B50;
    header->version                  = 0x1003;
    header->version_e                = 0x1003;
    header->bit_flag                 = 0x0000;
    header->compression_method       = 0x0000;
    header->file_time                = 0x0000;
    header->file_date                = 0x0000;
    header->crc_32                   = crc32(input);
    header->compressed_size          = getFileSize(input);
    header->uncompressed_size        = getFileSize(input);
    header->file_name_length         = strlen(file_name);
    header->extra_field_length       = 0;
    header->file_comment_length      = 0;
    header->disk_number_start        = 0;
    header->internal_file_attributes = 0;
    header->external_file_attributes = 0;
    header->relative_offset          = 0;
    for (int i = 0; i < strlen(file_name); i++) {
        header->file_name[i] = file_name[i];
    }

    if (fpos_b == false) {
        fpos_b = true;
        fgetpos(output, &fpos);
    }

    fwrite(header, sizeof(central_directory_file_header) + strlen(file_name), 1, output);

    fclose(input);
    free(header);

    total++;
    size += sizeof(central_directory_file_header) + strlen(file_name);
}

void addEndOfCentralDirectoryRecord(FILE *output)
{
    end_of_central_directory_record *record;

    record = (end_of_central_directory_record *)malloc(sizeof(end_of_central_directory_record));

    record->signature                 = 0x06054B50;
    record->disk                      = 0;
    record->directory                 = 0;
    record->total_number_of_disk      = total;
    record->total_number_of_directory = total;
    record->size                      = size;
    record->offset                    = fpos;
    record->comment_length            = 0;

    fwrite(record, sizeof(central_directory_file_header), 1, output);

    free(record);
}

static void initZipHeader(FILE *output, char *file_name, FILE *input)
{
    local_file_header *header;

    header = (local_file_header *)malloc(sizeof(local_file_header) + strlen(file_name) - 1);

    header->signature          = 0x04034B50;
    header->version            = 0x1003;
    header->bit_flag           = 0x0000;
    header->compression_method = 0x0000;
    header->file_time          = 0x0000;
    header->file_date          = 0x0000;
    header->crc_32             = crc32(input);
    header->compressed_size    = getFileSize(input);
    header->uncompressed_size  = getFileSize(input);
    header->file_name_length   = strlen(file_name);
    header->extra_field_length = 0;
    for (int i = 0; i < strlen(file_name); i++) {
        header->file_name[i] = file_name[i];
    }

    fwrite(header, 1, sizeof(local_file_header) + strlen(file_name), output);

    free(header);
}

static long getFileSize(FILE *file)
{
    long ans;

    fseek(file, 0, SEEK_END);
    //fgetpos(file, &ans);
    ans = ftell(file);
    fseek(file, 0, SEEK_SET);

    return ans;
}

static void fileCopy(FILE *output, FILE *input)
{
    size_t size;
    uint8_t data[1024];

    fseek(input, 0, SEEK_SET);

    size = fread(data, 1, 1024, input);
    while( size != 0 ) {
        fwrite(data, 1, size, output);
        size = fread(data, 1, 1024, input);
    }

    fseek(input, 0, SEEK_SET);
}
