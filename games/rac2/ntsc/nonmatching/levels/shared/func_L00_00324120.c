#include "common.h"

extern u8 D_00190000[];

int func_L00_00324120(s32 value) {
    u8 *data = D_00190000 - 0x61E0;

    if (*(s32 *)(data + 0x229C) != 3) {
        if (*(s32 *)(data + 0x2294) == 0x1B)
            goto compare_588;
        if (*(short *)(data + 0x34E) != 0)
            return 0;
        return *(s32 *)(data + 0x33C) == value;
    }

compare_588:
    return *(s32 *)(data + 0x588) == value;
}
