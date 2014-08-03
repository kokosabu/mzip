#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zip.h"

static void initZipHeader(FILE *output, char *file_name);

void addZip(FILE *output, char *file_name)
{
    initZipHeader(output, file_name);
}

static void initZipHeader(FILE *output, char *file_name)
{
    local_file_header *header;

    header = (local_file_header *)malloc(sizeof(local_file_header) + strlen(file_name) - 1);

    header->signature          = 0x0403B50;
    header->version            = 0x1003;
    header->bit_flag           = 0x0000;
    header->compression_method = 0x0000;
    header->file_time          = 0x0000;
    header->file_date          = 0x0000;
    header->crc_32             = 0;
    header->compressed_size    = 0;
    header->uncompressed_size  = 0;
    header->file_name_length   = strlen(file_name);
    header->extra_field_length = 0;
    for (int i = 0; i < strlen(file_name); i++) {
        header->file_name[i] = file_name[i];
    }

    fwrite(header, sizeof(local_file_header) + strlen(file_name) - 1, 1, output);

    free(header);
}
