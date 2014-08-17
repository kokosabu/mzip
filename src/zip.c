#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include "zip.h"
#include "crc32.h"

static uint16_t total  = 0;
static uint32_t size   = 0;
static fpos_t   fpos   = 0;
static bool     fpos_b = false;

static void initZipHeader(FILE *output, char *file_name, FILE *input);
static void fileCopy(FILE *output, FILE *input);
static long getFileSize(FILE *file);
static uint16_t getFileTime(char *file_name);
static uint16_t getFileDate(char *file_name);

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
    header = (central_directory_file_header *)malloc(sizeof(central_directory_file_header));
    header->file_name    = (char *)malloc(strlen(file_name));
    header->extra_field  = NULL;
    header->file_comment = NULL;

    header->signature                = 0x02014B50;
    header->version                  = 0x000a;
    header->version_e                = 0x000a;
    header->bit_flag                 = 0x0000;
    header->compression_method       = 0x0000;
    header->file_time                = getFileTime(file_name);
    header->file_date                = getFileDate(file_name);
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
    strncpy(header->file_name, file_name, strlen(file_name));

    if (fpos_b == false) {
        fpos_b = true;
        fgetpos(output, &fpos);
    }

    fwrite(header, sizeof(central_directory_file_header) - sizeof(header->file_name) - sizeof(header->extra_field) -sizeof(header->file_comment), 1, output);
    fwrite(header->file_name, strlen(file_name), 1, output);

    fclose(input);
    free(header->file_name);
    free(header);

    total++;
    size += sizeof(central_directory_file_header) - sizeof(header->file_name) - sizeof(header->extra_field) - sizeof(header->file_comment) + strlen(file_name);
}

void addEndOfCentralDirectoryRecord(FILE *output)
{
    end_of_central_directory_record *record;

    record = (end_of_central_directory_record *)malloc(sizeof(end_of_central_directory_record));
    record->file_comment = NULL;

    record->signature                 = 0x06054B50;
    record->disk                      = 0;
    record->directory                 = 0;
    record->total_number_of_disk      = total;
    record->total_number_of_directory = total;
    record->size                      = size;
    record->offset                    = fpos;
    record->comment_length            = 0;

    fwrite(record, sizeof(end_of_central_directory_record) - sizeof(record->file_comment), 1, output);

    free(record);
}

static void initZipHeader(FILE *output, char *file_name, FILE *input)
{
    local_file_header *header;

    header = (local_file_header *)malloc(sizeof(local_file_header));
    header->file_name   = (char *)malloc(strlen(file_name));
    header->extra_field = NULL;

    header->signature          = 0x04034B50;
    header->version            = 0x000a;
    header->bit_flag           = 0x0000;
    header->compression_method = 0x0000;
    header->file_time          = getFileTime(file_name);
    header->file_date          = getFileDate(file_name);
    header->crc_32             = crc32(input);
    header->compressed_size    = getFileSize(input);
    header->uncompressed_size  = getFileSize(input);
    header->file_name_length   = strlen(file_name);
    header->extra_field_length = 0;
    strncpy(header->file_name, file_name, strlen(file_name));

    fwrite(header, 1, sizeof(local_file_header) - sizeof(header->file_name) - sizeof(header->extra_field), output);
    fwrite(header->file_name, 1, strlen(file_name), output);

    free(header->file_name);
    free(header);
}

static long getFileSize(FILE *file)
{
    long ans;

    fseek(file, 0, SEEK_END);
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

static uint16_t getFileTime(char *file_name)
{
    struct stat st_file;
    uint16_t ans;
    struct tm *local;

    stat(file_name, &st_file);
    local = localtime(&(st_file.st_mtimespec.tv_sec));

    ans = 0;
    ans |= (local->tm_hour     & 0x1F) << 11;
    ans |= (local->tm_min      & 0x3F) <<  5;
    ans |= (local->tm_sec / 2) & 0x1F;

    return ans;
}

static uint16_t getFileDate(char *file_name)
{
    struct stat st_file;
    uint16_t ans;
    struct tm *local;

    stat(file_name, &st_file);
    local = localtime(&(st_file.st_mtimespec.tv_sec));

    ans = 0;
    ans |= ((local->tm_year - 80) & 0x7F) << 9;
    ans |= ((local->tm_mon  +  1) & 0xF)  << 5;
    ans |=   local->tm_mday       & 0x1F;

    return ans;
}
