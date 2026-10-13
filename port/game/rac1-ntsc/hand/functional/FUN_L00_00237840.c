/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002381D0.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
typedef struct {
    char pad0[0x8];
    int unk08;
    int *unk0C;
    char pad10[0x38];
    short unk48;
    short unk4A;
    char pad4C[0x4];
    int unk50;
    int unk54;
    int w;
    int h;
    int flags;
    char pad64[0x8];
    int unk6C;
    unsigned char cnt[4];
    int unk74;
    int unk78;
    int unk7C;
    void *unk80;
} HudElem_c237840;

/* NON_MATCHING FUN_L00_00237840 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 672 / retail 680, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Cannot land as written (#define in a candidate): rewrite that in plain C first.
 * What the last attempts found:
 *   FUN_L00_00237840: draws a HUD gauge (shared hud): stores the element's w/h from two gp globals, anchors it wi
 *   Best p4/p6/p7: 668 bytes vs retail 680, structure and all multiplies match (separate temps h1/h2/h3 = HH*0xEA0
 */
extern short D_L00_0015F760;
extern short D_L00_0015F764;
extern void func_00200E08(int, int, int, int, long, long);
extern int func_001FF960(int, int);
extern void func_L00_00235AD8(HudElem_c237840 *, int *, int *, int, int);
extern void func_L00_0023BAC0(int, int, int, int, int, int);
#define HW (*(int *)&D_L00_0015F760)
#define HH (*(int *)&D_L00_0015F764)

/* Draws a HUD gauge frame: a background bar and tick marks from the element's position and size. */
int FUN_L00_00237840(HudElem_c237840 *e) {
    int pos[2];
    int x0, x1, y, i, hs;

    pos[0] = e->unk50;
    pos[1] = e->unk54;
    e->w = HW;
    e->h = HH;
    func_L00_00235A70(e, &pos[0], &pos[1]);
    func_L00_00235AD8(e, &pos[0], &pos[1], e->unk6C, 0);
    hs = HH * 0xEA0;
    x0 = ((pos[0] + e->unk48) << 4) + (HW * 0xD0) / 64;
    y = ((pos[1] + e->unk4A) << 4) + (HH * 0xA0) / 256;
    x1 = x0 + (HW * 0x260) / 64;
    func_00200E08(x0, y, x1, y + hs / 256, 0x80000000, 1);
    y = ((pos[1] + e->unk4A) << 4) + (HH * 0xF40) / 256;
    func_00200E08(x0, y, x1, y - (e->unk74 * hs) / (e->unk08 << 8), 0x80829E00, 1);
    for (i = 1; i < e->unk74; i++) {
        hs = HH * 0xEA0;
        y = ((pos[1] + e->unk4A) << 4) + (HH * 0xF40) / 256 - (i * hs) / (e->unk08 << 8) - 0x10;
        func_00200E08(x0, y, x1, y + 0x20, 0x80000000, 1);
    }
    func_L00_0023BAC0(func_001FF960(*(int *)e, 0), pos[0] + e->unk48, pos[1] + e->unk4A, HW, HH, 0x80);
    return e->w;
}
