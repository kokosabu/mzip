#ifndef __ZIP_H__
#define __ZIP_H__

#include <stdint.h>

typedef struct _local_file_header local_file_header;
struct _local_file_header {
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
} __attribute__((packed));

typedef struct _central_directory_file_header central_directory_file_header;
struct _central_directory_file_header {
    uint32_t signature;                /* central file header signature   */
    uint16_t version;                  /* version made by                 */
    uint16_t version_e;                /* version needed to extract       */
    uint16_t bit_flag;                 /* general purpose bit flag        */
    uint16_t compression_method;       /* compression method              */
    uint16_t file_time;                /* last mod file time              */
    uint16_t file_date;                /* last mod file date              */
    uint32_t crc_32;                   /* crc-32                          */
    uint32_t compressed_size;          /* compressed size                 */
    uint32_t uncompressed_size;        /* uncompressed size               */
    uint16_t file_name_length;         /* file name length                */
    uint16_t extra_field_length;       /* extra field length              */
    uint16_t file_comment_length;      /* file comment length             */
    uint16_t disk_number_start;        /* disk number start               */
    uint16_t internal_file_attributes; /* internal file attributes        */
    uint32_t external_file_attributes; /* external file attributes        */
    uint32_t relative_offset;          /* relative offset of local header */
    char     file_name[0];             /* file name                       */
#if 0
    char     extra_field[0];           /* extra field                     */
    char     file_comment[0];          /* file comment                    */
#endif
} __attribute__((packed));

typedef struct {
    uint32_t signature;                 /* end of central dir signature                                                  */
    uint16_t disk;                      /* number of this disk                                                           */
    uint16_t directory;                 /* number of the disk with the start of the central directory                    */
    uint16_t total_number_of_disk;      /* total number of entries in the central directory on this disk                 */
    uint16_t total_number_of_directory; /* total number of entries in the central directory                              */
    uint32_t size;                      /* size of the central directory                                                 */
    uint32_t offset;                    /* offset of start of central directory with respect to the starting disk number */
    uint16_t comment_length;            /* .ZIP file comment length                                                      */
    uint8_t  file_comment[0];           /* .ZIP file comment                                                             */
} end_of_central_directory_record;

typedef struct {
    uint32_t crc_32;            /* crc-32            */
    uint32_t compressed_size;   /* compressed size   */
    uint32_t uncompressed_size; /* uncompressed size */
} Data_descriptor;

void addZip(FILE *output, char *file_name);
void make_crc_table(void);
void addCentralDirectoryHeader(FILE *output, char *file_name);
void addEndOfCentralDirectoryRecord(FILE *output);

#endif /* __ZIP_H__ */
