#include "common.h"

extern u8 * volatile * volatile D_L00_001B1AFC __attribute__((sda));

void func_L00_003296C0(s32 enabled, s32 index, s32 value) {
    s32 saved_index = index;
    u8 * volatile *table;
    u8 *record;
    s32 byte_offset;

    if (enabled == 0)
        return;
    table = D_L00_001B1AFC;
    record = table[saved_index];
    byte_offset = record[4] << 3;
    *(s32 *)(record + byte_offset) = enabled;
    table[saved_index][byte_offset + 5] = (u8)value;
    table = D_L00_001B1AFC;
    *(short *)(table[saved_index] + byte_offset + 6) = 0;
    ++table[saved_index][4];
}
