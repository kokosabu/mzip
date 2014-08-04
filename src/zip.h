#ifndef __ZIP_H__
#define __ZIP_H__

#include <stdint.h>

typedef struct {
    uint32_t signature;             /* local file header signature */
    uint16_t version;               /* version needed to extract   */
    uint16_t bit_flag;              /* general purpose bit flag    */
    uint16_t compression_method;    /* compression method          */
    uint16_t file_time;             /* last mod file time          */
    uint16_t file_date;             /* last mod file date          */
    uint32_t crc_32;                /* crc-32                      */
    uint32_t compressed_size;       /* compressed size             */
    uint32_t uncompressed_size;     /* uncompressed size           */
    uint16_t file_name_length;      /* file name length            */
    uint16_t extra_field_length;    /* extra field length          */
    char     file_name[0];          /* file name                   */
#if 0
    char     extra_field[0];        /* extra field                 */
#endif
} local_file_header;

typedef struct {
    uint32_t crc_32;            /* crc-32            */
    uint32_t compressed_size;   /* compressed size   */
    uint32_t uncompressed_size; /* uncompressed size */
} Data_descriptor;

void addZip(FILE *output, char *file_name);
void make_crc_table(void);

#endif /* __ZIP_H__ */
