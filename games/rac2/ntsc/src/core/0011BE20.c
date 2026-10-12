#include "common.h"
#include "structs.h"

extern u8 r1_D_00154B80[];

struct TtyInitState {
    s32 unk0;
    volatile s32 unk4;
    volatile s32 unk8;
    volatile s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
};

struct TtyMmio {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u8 unk6;
    u8 unk7;
    u32 unk8;
};

extern struct TtyInitState r1_D_00154B50;

extern u8 r1_D_00154CC0[];

extern s32 r1_func_00118D80();

extern s32 r1_func_00119868_tty(int) __asm__("func_0011BAC8");

extern s32 r1_func_00119718();

extern void func_0011BB38();

/* rac1-decomp src/core/00119868.c: func_00119CC8 */
/* Opens the TTY DECI2 endpoint and initializes its uncached packet headers. */
s32 func_0011BE20(void) {
    struct TtyInitState *state = &r1_D_00154B50;
    struct TtyMmio *p;
    struct TtyMmio *q;
    u32 mask;
    s32 baud;
    s32 cmd;

    r1_func_00118D80(0);
    *(volatile s32 *)&state->unk0 = r1_func_00119718(0x210, state, &func_0011BB38);
    if (state->unk0 < 0) {
        return 0;
    }
    state->unkC = 0;
    mask = 0x20000000;
    q = (struct TtyMmio *)((u32)r1_D_00154CC0 | mask);
    state->unk4 = 0;
    p = (struct TtyMmio *)((u32)r1_D_00154B80 | mask);
    state->unk8 = 0;
    state->unk14 = (s32)q;
    state->unk10 = (s32)p;
    baud = 0x210;
    cmd = 0x45;
    p->unk2 = 0;
    p->unk4 = baud;
    p->unk6 = cmd;
    p->unk7 = 0x48;
    p->unk8 = 0;
    state->unk18 = r1_func_00119868_tty(0x100);
    return 1;
}

INCLUDE_ASM("asm/core", func_0011BEDC);
