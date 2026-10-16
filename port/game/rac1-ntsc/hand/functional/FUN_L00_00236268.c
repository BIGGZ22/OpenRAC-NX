/* Carried from the PAL decompilation's nonmatching/shared/func_L00_00236BF8.c (games/rac1) through the version
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
} HudElem_c236268;

/* NON_MATCHING FUN_L00_00236268 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 496 / retail 492, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Fills 100 16-byte hud particle records at D_L00_0017E660 (x, y from func_002132A8 ranges, two drift floats as 
 *   Best try p4.c (struct array, hi = lo + span before the calls): same instructions but 508 vs 492 bytes: ours ke
 *   Flat float[] indexing (p6) gives 460 bytes (everything reduced to one giv); 2D/struct/pointer forms all give 5
 *   x02 round (p8-p12): nothing beat best.c (504/492). Tried: hi local (508), explicit pointer q for [0]/[3] (p11,
 */
extern float func_002132A8(float, float);
extern float D_L00_0017E660[][4];
extern short D_L00_0015F750;
extern short D_L00_0015F754;
extern short D_L00_0015F758;
extern short D_L00_0015F75C;

/* fills 100 hud particle records with randomised positions and drift */
void FUN_L00_00236268(HudElem_c236268 *e) {
    int i;
    float v, lo, span;
    int n;
    float *q = &D_L00_0017E660[0][3];

    func_L00_00235D80(e);
    for (i = 0; i < 100; i++, q += 4) {
        v = i < 10 ? *(float *)&D_L00_0015F750 : i < 20 ? *(float *)&D_L00_0015F754 : *(float *)&D_L00_0015F758;
        n = *(int *)&D_L00_0015F75C;
        span = (232.0f - (v + v)) / (float)n;
        lo = (v - 116.0f) + (float)(i % n) * span;
        q[-3] = func_002132A8(v - 17.0f, 17.0f - v);
        D_L00_0017E660[i][1] = func_002132A8(lo, lo + span);
        D_L00_0017E660[i][2] = func_002132A8(-0.4f, 0.4f);
        *q = func_002132A8(-0.6f, 0.6f);
        D_L00_0017E660[i][2] += func_002132A8(-0.4f, 0.4f);
        *q += func_002132A8(-0.6f, 0.6f);
        D_L00_0017E660[i][2] += func_002132A8(-0.4f, 0.4f);
        *q += func_002132A8(-0.6f, 0.6f);
    }
}
