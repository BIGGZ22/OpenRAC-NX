/* Carried from the PAL decompilation's src/overlays/shared/hud_00235960.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
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
int func_L00_00235A70(HudElem_c237840 *rec, int *x, int *y);
/* FUN_L00_00237840 -- src/overlays/shared/hud_00235960.c
 * Draws a HUD gauge frame. EXACT: e->h is copied with __builtin_memcpy so
 * the D_L00_0015F764 load is a scalar MEM (no anti-dep on the e->w/e->h
 * stores), which moves it later in sched1 and gives it $a3 in local-alloc.
 */
extern short D_L00_0015F760;
extern short D_L00_0015F764;
extern void func_00200E08(int, int, int, int, long, long);
extern int func_001FF960(int, int);
extern void func_L00_00235AD8(HudElem_c237840 *, int *, int *, int, int);
extern void func_L00_0023BAC0(int, int, int, int, int, int);



/* Draws a HUD gauge frame. */
int FUN_L00_00237840(HudElem_c237840 *e) {
    int pos[2];
    int left, top, right, bottom, w, h, bh, step, i;
    {
        int ww = *(int *)&D_L00_0015F760;
        pos[0] = e->unk50;
        pos[1] = e->unk54;
        e->w = ww;
        {
            int *src = (int *)&D_L00_0015F764;
            int *dst = &e->h;
            __builtin_memcpy(dst, src, 4);
        }
    }
    func_L00_00235A70(e, &pos[0], &pos[1]);
    func_L00_00235AD8(e, &pos[0], &pos[1], e->unk6C, 0);
    w = *(int *)&D_L00_0015F760 * 0xD0;
    h = *(int *)&D_L00_0015F764 * 0xA0;
    bh = *(int *)&D_L00_0015F760 * 0x260;
    bottom = *(int *)&D_L00_0015F764 * 0xEA0;
    left = (pos[0] + e->unk48) << 4;
    top = (pos[1] + e->unk4A) << 4;
    left += w / 64;
    top += h / 256;
    right = left + bh / 64;
    bottom = top + bottom / 256;
    func_00200E08(left, top, right, bottom, 0x80000000, 1);
    {
        int st = *(int *)&D_L00_0015F764 * 0xEA0;
        h = *(int *)&D_L00_0015F764;
        top = (pos[1] + e->unk4A) << 4;
        top += (h * 0xF40) / 256;
        bh = (e->unk74 * st) / (e->unk08 << 8);
        func_00200E08(left, top, right, top - bh, 0x80829E00, 1);
    }
    i = 1;
    if (i < e->unk74) do {
        int hh = *(int *)&D_L00_0015F764;
        top = (pos[1] + e->unk4A) << 4;
        top += (hh * 0xF40) / 256;
        step = *(int *)&D_L00_0015F764 * 0xEA0;
        step = i * step / (e->unk08 << 8);
        i++;
        top -= step;
        top -= 16;
        func_00200E08(left, top, right, top + 32, 0x80000000, 1);
    } while (i < e->unk74);
    w = func_001FF960(*(int *)e, 0);
    func_L00_0023BAC0(w, pos[0] + e->unk48, pos[1] + e->unk4A, *(int *)&D_L00_0015F760, *(int *)&D_L00_0015F764, 0x80);
    return e->w;
}
