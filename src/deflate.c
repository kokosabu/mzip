#include <stdint.h>
#include "deflate.h"

#define BIT_00000001 (0x01)
#define BIT_00000010 (0x02)
#define BIT_00000100 (0x04)
#define BIT_00001000 (0x08)
#define BIT_00010000 (0x10)
#define BIT_00100000 (0x20)
#define BIT_01000000 (0x40)
#define BIT_10000000 (0x80)

static const uint8_t bit_table[8] = {
    BIT_00000001,
    BIT_00000010,
    BIT_00000100,
    BIT_00001000,
    BIT_00010000,
    BIT_00100000,
    BIT_01000000,
    BIT_10000000
};

static void bit_set(uint8_t *to, uint8_t bitpos, uint8_t src)
{
    *to |= ((src & 0x01) << bitpos);
}
