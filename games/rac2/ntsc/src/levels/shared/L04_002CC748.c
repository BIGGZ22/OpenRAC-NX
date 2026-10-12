#include "include_asm.h"

typedef int s32;

typedef unsigned int u32;

typedef struct {
    u32 unknown;
    s32 data_offset;
    s32 length;
    s32 extra;
} PackedHeaderCountView;

INCLUDE_ASM("asm/levels", func_L04_002CC748);

INCLUDE_ASM("asm/levels", func_L04_002CE810);

INCLUDE_ASM("asm/levels", func_L04_002CE870);

INCLUDE_ASM("asm/levels", func_L04_002D91B8);

INCLUDE_ASM("asm/levels", func_L04_002D9780);

INCLUDE_ASM("asm/levels", func_L04_002DB970);

INCLUDE_ASM("asm/levels", func_L04_002DBE70);

INCLUDE_ASM("asm/levels", func_L04_002DCE78);

INCLUDE_ASM("asm/levels", func_L04_002FB920);

INCLUDE_ASM("asm/levels", func_L04_0030C4C0);

INCLUDE_ASM("asm/levels", func_L04_0030C948);

INCLUDE_ASM("asm/levels", func_L04_0030CC70);

INCLUDE_ASM("asm/levels", func_L04_0030EDE0);

INCLUDE_ASM("asm/levels", func_L04_0030FA30);

INCLUDE_ASM("asm/levels", func_L04_0031D630);

INCLUDE_ASM("asm/levels", func_L04_0032F1D8);

INCLUDE_ASM("asm/levels", func_L04_0032F340);

INCLUDE_ASM("asm/levels", func_L04_0032F3C0);

INCLUDE_ASM("asm/levels", func_L04_0032F62C);

INCLUDE_ASM("asm/levels", func_L04_0032F994);

INCLUDE_ASM("asm/levels", func_L04_0033C420);

INCLUDE_ASM("asm/levels", func_L04_0033CB08);

/* sched1 hoists the last statement of */
void func_L04_0033CB88(char *p)
{
    *(int *)(p + 0) = 640;
    *(float *)(p + 8) = 0.5f;
    *(int *)(p + 20) = 0;
    *(float *)(p + 4) = 1.0f;
    *(float *)(p + 12) = 0.5f;
    *(int *)(p + 16) = 0;
    *(float *)(p + 36) = 2.7f;
    *(int *)(p + 24) = 0;
    *(float *)(p + 32) = 0.3f;
    *(float *)(p + 28) = 0.3f;
    *(float *)(p + 40) = 1.57f;
    *(float *)(p + 44) = 0.1f;
}

INCLUDE_ASM("asm/levels", func_L04_0033CC00);

INCLUDE_ASM("asm/levels", func_L04_0033D0C0);

INCLUDE_ASM("asm/levels", func_L04_0033D158);

INCLUDE_ASM("asm/levels", func_L04_0033D4B0);

INCLUDE_ASM("asm/levels", func_L04_003429F8);

INCLUDE_ASM("asm/levels", func_L04_00343498);

INCLUDE_ASM("asm/levels", func_L04_00344568);

INCLUDE_ASM("asm/levels", func_L04_00347AB8);

INCLUDE_ASM("asm/levels", func_L04_0034B9B0);

INCLUDE_ASM("asm/levels", func_L04_003524D0);

INCLUDE_ASM("asm/levels", func_L04_00358868);

INCLUDE_ASM("asm/levels", func_L04_00358AE8);

INCLUDE_ASM("asm/levels", func_L04_0035F3E0);

INCLUDE_ASM("asm/levels", func_L04_00362768);

INCLUDE_ASM("asm/levels", func_L04_00362940);

INCLUDE_ASM("asm/levels", func_L04_00362A50);

INCLUDE_ASM("asm/levels", func_L04_00363690);

INCLUDE_ASM("asm/levels", func_L04_00363CA0);

INCLUDE_ASM("asm/levels", func_L04_00365300);

INCLUDE_ASM("asm/levels", func_L04_00365B48);

INCLUDE_ASM("asm/levels", func_L04_00378838);

INCLUDE_ASM("asm/levels", func_L04_0039FA08);

INCLUDE_ASM("asm/levels", func_L04_0039FC28);

INCLUDE_ASM("asm/levels", func_L04_0039FEA8);

INCLUDE_ASM("asm/levels", func_L04_0039FEF0);

INCLUDE_ASM("asm/levels", func_L04_003A0008);

INCLUDE_ASM("asm/levels", func_L04_003A00E8);

INCLUDE_ASM("asm/levels", func_L04_003A02A0);

INCLUDE_ASM("asm/levels", func_L04_003A0C40);

INCLUDE_ASM("asm/levels", func_L04_003A0CE8);

INCLUDE_ASM("asm/levels", func_L04_003A0FC8);

INCLUDE_ASM("asm/levels", func_L04_003A11F8);

INCLUDE_ASM("asm/levels", func_L04_003A1240);

INCLUDE_ASM("asm/levels", func_L04_003A14B8);

/* returns the packed header's signed rounded sixteen-byte block count in word units */
s32 func_L04_003A1540(const PackedHeaderCountView *header) {
    s32 fixed = header->extra + 16;
    s32 total = header->data_offset + fixed + header->length;
    return ((total + 15) / 16) * 4;
}

INCLUDE_ASM("asm/levels", func_L04_003A1578);

INCLUDE_ASM("asm/levels", func_L04_003A15C0);

INCLUDE_ASM("asm/levels", func_L04_003A1868);

INCLUDE_ASM("asm/levels", func_L04_003A19E8);

INCLUDE_ASM("asm/levels", func_L04_003A1A48);

INCLUDE_ASM("asm/levels", func_L04_003A1A78);

INCLUDE_ASM("asm/levels", func_L04_003A1C98);

INCLUDE_ASM("asm/levels", func_L04_003A1FA0);

INCLUDE_ASM("asm/levels", func_L04_003A1FE0);

INCLUDE_ASM("asm/levels", func_L04_003A21F8);

INCLUDE_ASM("asm/levels", func_L04_003A5C08);

INCLUDE_ASM("asm/levels", func_L04_003B3168);

INCLUDE_ASM("asm/levels", func_L04_003B3580);

INCLUDE_ASM("asm/levels", func_L04_003C3BA0);

INCLUDE_ASM("asm/levels", func_L04_003C4050);

INCLUDE_ASM("asm/levels", func_L04_003C4858);

INCLUDE_ASM("asm/levels", func_L04_003C49C0);

INCLUDE_ASM("asm/levels", func_L04_003C6FE8);

INCLUDE_ASM("asm/levels", func_L04_003C70C8);
