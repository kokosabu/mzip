#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
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
static uint16_t get_FileTime(char *file_name);
static uint16_t get_FileDate(char *file_name);

const unsigned long dayofm[ ] = {    /* 閏年でない年の各月の日数 */
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const unsigned long dayoflm[ ] = {    /* 閏年の各月の日数 */
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const char *monthstr[ ] = {    /* 各月の名前 */
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Des"
};

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
    header->file_time                = get_FileTime(file_name);
    header->file_date                = get_FileDate(file_name);
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
    header->file_time          = get_FileTime(file_name);
    header->file_date          = get_FileDate(file_name);
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

static uint16_t get_FileTime(char *file_name)
{
    struct stat st_file;
    time_t  timer, work, sec, min, hour, day;
    int year = 1970;
    int month = 1;
    uint16_t ans;

    stat(file_name, &st_file);

    work = st_file.st_mtimespec.tv_sec; 

    sec = work % 60;        /* 秒を割り出す */
    work -= sec;            /* 秒を引く */
    work = work / 60;       /* 分の単位に変換 */

    min = work % 60;        /* 分を割り出す */
    work -= min;            /* 分を引く */
    work = work / 60;       /* 時の単位に変換 */

    hour = work % 24;       /* 時を割り出す */
    work -= hour;           /* 時を引く */

    day = work / 24;        /* 1970 年からの経過日数を割り出す */

    /* このループを抜けると day には今年の経過日数が残る */
    while (day > 366) {            /* 1970 年から何年経過したか調べる */
        if (!(year % 4) && ((year % 100) || !(year % 400))) {
            day -= 366;            /* 366 を引き */
        } else {                      /* 閏年でなければ */
            day -= 365;            /* 365 を引き */
        }
        year++;                    /* 西暦を 1 増やす */
    }

    day++;                         /* 1 月 1 日は 0 だから */
    /* day
     * には最初は今年の経過日数が入っている。各月の日数を順次引くことにより
     * */
    /* このループを抜けると day にはその月の経過日数 ( 日付 ) が残る */
    while (1) {                 
        if (!(year % 4) && ((year % 100) || !(year % 400))) {
            if (day <= dayoflm[month -1])       /* 月の日数より day が少なければ（等号が抜けていました。（ＨＮ）James Bond さんご指摘ありがとうございました。） */
                break;
            else {                             /* 月の日数より day が多ければ */
                day -= dayoflm[month -1];      /* 月の日数を引き */
                month++;                       /* 月を 1 増やす */
            }
        }
        if (!(!(year % 4) && ((year % 100) || !(year % 400)))) {               /* もし閏年でなければ */
            if (day <= dayofm[month -1])        /* 以下同上 */
                break;
            else {
                day -= dayofm[month -1];
                month++;
            }
        }
    }

    ans = 0;
    ans |= (hour & 0x1F) << 11;
    ans |= (min & 0x3F)   << 5;
    ans |= (sec / 2) & 0x1F;

    printf("%x\n", ans);
    //ビット割り当て
    // 12:01100 21:10101
    //%1111100000000000 = 0～23 [時]
    //%0000011111100000 = 0～59 [分]
    //%0000000000011111 = 0～29 [×2秒] ⇒1で2秒分を表す
    // 654e : 0110 0101 0100 1110 - Time
    //        01100 101010 01110
    //        12    42     14->28
    // a90a : 1010 1001 0000 1010 - Date
    // a90a : 1010100 1000 01010
    //        1 2 4 8 16 32 64
    // miura-no-MacBook-Pro:src miura$ ls -l hoge.txt
    // -rw-r--r--  1 miura  staff  8  8 10 21:42 hoge.txt

    return ans;
}

static uint16_t get_FileDate(char *file_name)
{
    struct stat st_file;
    time_t  timer, work, sec, min, hour, day;
    int year = 1970;
    int month = 1;
    uint16_t ans;

    stat(file_name, &st_file);

    work = st_file.st_mtimespec.tv_sec; 

    sec = work % 60;        /* 秒を割り出す */
    work -= sec;            /* 秒を引く */
    work = work / 60;       /* 分の単位に変換 */

    min = work % 60;        /* 分を割り出す */
    work -= min;            /* 分を引く */
    work = work / 60;       /* 時の単位に変換 */

    hour = work % 24;       /* 時を割り出す */
    work -= hour;           /* 時を引く */

    day = work / 24;        /* 1970 年からの経過日数を割り出す */

    /* このループを抜けると day には今年の経過日数が残る */
    while (day > 366) {            /* 1970 年から何年経過したか調べる */
        if (!(year % 4) && ((year % 100) || !(year % 400))) {
            day -= 366;            /* 366 を引き */
        } else {                      /* 閏年でなければ */
            day -= 365;            /* 365 を引き */
        }
        year++;                    /* 西暦を 1 増やす */
    }

    day++;                         /* 1 月 1 日は 0 だから */
    /* day
     * には最初は今年の経過日数が入っている。各月の日数を順次引くことにより
     * */
    /* このループを抜けると day にはその月の経過日数 ( 日付 ) が残る */
    while (1) {                 
        if (!(year % 4) && ((year % 100) || !(year % 400))) {
            if (day <= dayoflm[month -1])       /* 月の日数より day が少なければ（等号が抜けていました。（ＨＮ）James Bond さんご指摘ありがとうございました。） */
                break;
            else {                             /* 月の日数より day が多ければ */
                day -= dayoflm[month -1];      /* 月の日数を引き */
                month++;                       /* 月を 1 増やす */
            }
        }
        if (!(!(year % 4) && ((year % 100) || !(year % 400)))) {               /* もし閏年でなければ */
            if (day <= dayofm[month -1])        /* 以下同上 */
                break;
            else {
                day -= dayofm[month -1];
                month++;
            }
        }
    }

    ans = 0;
    ans |= ((year-1980) & 0x7F) << 9;
    ans |= (month & 0xF)   << 5;
    ans |= day & 0x1F;
    printf("%x\n", ans);
    // %11111110 00000000 = 0～   [＋1980年] ⇒西暦1980年からの経過年
    // %00000001 11100000 = 1～12 [月]
    // %00000000 00011111 = 1～31 [日]

    return ans;
}
