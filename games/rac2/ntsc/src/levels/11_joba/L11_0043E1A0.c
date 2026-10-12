#include "include_asm.h"

typedef float f32;

typedef unsigned char u8;

typedef int s32;

/* Dispatches an indexed polygon query through the already integrated helper. */
typedef struct { s32 field00; u8 remainder[12]; } JobaPolygonHeaderView;

extern JobaPolygonHeaderView *D_L11_001FF580[];

extern s32 func_002A8AF0(f32 *point, f32 *vertices, s32 count);

/* Consumes one positive counter selected by the measured class identifier. */
typedef struct {
    u8 prefix[0xe0];
    s32 fielde0, fielde4, fielde8, fieldec, fieldf0, fieldf4;
} JobaClassCounterView;

INCLUDE_ASM("asm/levels", func_L11_0043E1A0);

INCLUDE_ASM("asm/levels", func_L11_00440160);

INCLUDE_ASM("asm/levels", func_L11_004401C0);

INCLUDE_ASM("asm/levels", func_L11_00440238);

INCLUDE_ASM("asm/levels", func_L11_00440360);

INCLUDE_ASM("asm/levels", func_L11_00440790);

INCLUDE_ASM("asm/levels", func_L11_00440868);

INCLUDE_ASM("asm/levels", func_L11_00440970);

INCLUDE_ASM("asm/levels", func_L11_00440B28);

INCLUDE_ASM("asm/levels", func_L11_00440C80);

INCLUDE_ASM("asm/levels", func_L11_00440C90);

INCLUDE_ASM("asm/levels", func_L11_00440E18);

INCLUDE_ASM("asm/levels", func_L11_00441090);

INCLUDE_ASM("asm/levels", func_L11_00441108);

INCLUDE_ASM("asm/levels", func_L11_004411C8);

INCLUDE_ASM("asm/levels", func_L11_00441350);

INCLUDE_ASM("asm/levels", func_L11_004417A0);

INCLUDE_ASM("asm/levels", func_L11_004417A8);

INCLUDE_ASM("asm/levels", func_L11_004417E8);

/* returns zero for a missing state polygon index or dispatches its point query to the integrated crossing-parity helper */
s32 func_L11_00441808(u8 *object, f32 *point) {
    u8 *state = *(u8 **)(object + 0x68);
    s32 index = *(s32 *)(state + 0x164);
    JobaPolygonHeaderView *header;
    s32 result;
    if (index == -1) {
        result = 0;
    } else {
        header = D_L11_001FF580[index];
        result = func_002A8AF0(point, (f32 *)(header + 1), header->field00);
    }
    return result;
}

INCLUDE_ASM("asm/levels", func_L11_00441858);

/* consumes one positive signed state counter selected by a measured class identifier and reports success */
s32 func_L11_00441868(u8 *object, s32 class_id) {
    JobaClassCounterView *state = *(JobaClassCounterView **)(object + 0x68);
    switch (class_id) {
        case 0xd98:
            if (state->fielde0 > 0) { state->fielde0--; return 1; }
            break;
        case 0xdcf:
            if (state->fielde4 > 0) { state->fielde4--; return 1; }
            break;
        case 0xdf4:
            if (state->fielde8 > 0) { state->fielde8--; return 1; }
            break;
        case 0xdf7:
            if (state->fieldec > 0) { state->fieldec--; return 1; }
            break;
        case 0xdf9:
            if (state->fieldf0 > 0) { state->fieldf0--; return 1; }
            break;
        case 0xd37:
            if (state->fieldf4 > 0) { state->fieldf4--; return 1; }
            break;
    }
    return 0;
}

/* increments one signed state counter selected by a measured class identifier */
/* Increments the state counter selected by the measured class identifier. */
void func_L11_00441978(u8 *object, s32 class_id) {
    JobaClassCounterView *state = *(JobaClassCounterView **)(object + 0x68);
    switch (class_id) {
        case 0xd98: state->fielde0++; break;
        case 0xdcf: state->fielde4++; break;
        case 0xdf4: state->fielde8++; break;
        case 0xdf7: state->fieldec++; break;
        case 0xdf9: state->fieldf0++; break;
        case 0xd37: state->fieldf4++; break;
    }
}

INCLUDE_ASM("asm/levels", func_L11_00441A58);

INCLUDE_ASM("asm/levels", func_L11_00441D08);

INCLUDE_ASM("asm/levels", func_L11_00441FB8);

INCLUDE_ASM("asm/levels", func_L11_00442170);

INCLUDE_ASM("asm/levels", func_L11_004423F8);

INCLUDE_ASM("asm/levels", func_L11_004425C8);

INCLUDE_ASM("asm/levels", func_L11_00442710);

INCLUDE_ASM("asm/levels", func_L11_004427B8);

INCLUDE_ASM("asm/levels", func_L11_00442B60);

INCLUDE_ASM("asm/levels", func_L11_00442C80);

INCLUDE_ASM("asm/levels", func_L11_00442E38);

INCLUDE_ASM("asm/levels", func_L11_00443380);

INCLUDE_ASM("asm/levels", func_L11_004433D8);

INCLUDE_ASM("asm/levels", func_L11_00443498);

INCLUDE_ASM("asm/levels", func_L11_00443940);

INCLUDE_ASM("asm/levels", func_L11_00443CE0);

INCLUDE_ASM("asm/levels", func_L11_00443D78);

INCLUDE_ASM("asm/levels", func_L11_00444400);

INCLUDE_ASM("asm/levels", func_L11_004456A0);

INCLUDE_ASM("asm/levels", func_L11_00445990);

INCLUDE_ASM("asm/levels", func_L11_00445B30);

INCLUDE_ASM("asm/levels", func_L11_00445D28);

INCLUDE_ASM("asm/levels", func_L11_00445DF8);

INCLUDE_ASM("asm/levels", func_L11_00446020);

INCLUDE_ASM("asm/levels", func_L11_004461C0);

INCLUDE_ASM("asm/levels", func_L11_004461E8);
