#include "common.h"

extern u8 D_L00_00268E20;

u8 *func_L00_0032A0B8(s32 key, s32 offset) {
    u8 *entry = &D_L00_00268E20;
    entry += offset;
    if (*(s32 *)(entry + 4) != key)
        return 0;
    return entry + 8;
}
