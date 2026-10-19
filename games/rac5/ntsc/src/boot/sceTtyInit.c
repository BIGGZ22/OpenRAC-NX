/* RAC5 boot SCUS_976.15 - sceTtyInit @ 0x01ED2160.
 * Real extent 0x01ED2160..0x01ED221C (184 bytes + 4-byte nop; the next_label
 * interval runs to scePrintf but holds three unnamed helpers).
 * 2026-10-17 wave 11 lot b12.
 *
 * Provenance: Sony EE libkernl tty.o. Body structure taken from the R1PAL
 * decomp (/mnt/d/R1PAL/src/core/00119868.c, func_00119CC8) and re-expressed
 * against the RAC5 retail disassembly (call targets, store order, and the
 * uncached 0x20000000 aliases verified instruction by instruction).
 */
typedef signed int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

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

extern struct TtyInitState D_01FE3A90;
extern u8 D_01FE3AC0[];   /* 0x01FE3AC0: uncached packet header */
extern u8 D_01FE3C00[];   /* 0x01FE3C00: second uncached block */
extern s32 FlushCache(s32);
extern s32 sceDeci2Open(s32, void *, void *);
extern void sceTtyHandler(void);
extern s32 func_01ED1CD8(s32);

s32 sceTtyInit(void) {
    struct TtyInitState *state = &D_01FE3A90;
    struct TtyMmio *p;
    struct TtyMmio *q;

    FlushCache(0);
    *(volatile s32 *)&state->unk0 = sceDeci2Open(0x210, state, &sceTtyHandler);
    if (state->unk0 < 0) {
        return 0;
    }
    state->unkC = 0;
    state->unk4 = 0;
    state->unk8 = 0;
    q = (struct TtyMmio *)((u32)D_01FE3C00 | 0x20000000);
    p = (struct TtyMmio *)((u32)D_01FE3AC0 | 0x20000000);
    state->unk14 = (s32)q;
    state->unk10 = (s32)p;
    p->unk4 = 0x210;
    p->unk6 = 0x45;
    p->unk7 = 0x48;
    p->unk2 = 0;
    p->unk8 = 0;
    state->unk18 = func_01ED1CD8(0x100);
    return 1;
}
