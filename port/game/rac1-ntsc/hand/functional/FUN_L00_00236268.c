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
} HudElem_c236268;
void func_L00_00235D80(HudElem_c236268 *e);
extern float func_002132A8(float, float);
extern float D_L00_0017E660[][4];
extern short D_L00_0015F750;
extern short D_L00_0015F754;
extern short D_L00_0015F758;
extern short D_L00_0015F75C;



/* fills 100 hud particle records with randomised positions and drift */
void FUN_L00_00236268(HudElem_c236268 *e) {
    int i;
    float v, lo, span, hi;
    int n;

    func_L00_00235D80(e);
    for (i = 0; i < 100; i++) {
        v = *(float *)&D_L00_0015F750;
        if (i >= 10) {
            v = *(float *)&D_L00_0015F754;
            if (i >= 20) v = *(float *)&D_L00_0015F758;
        }
        n = *(int *)&D_L00_0015F75C;
        span = (232.0f - (v + v)) / (float)n;
        lo = (v - 116.0f) + (float)(i % n) * span;
        hi = lo + span;
        D_L00_0017E660[i][0] = func_002132A8(v - 17.0f, 17.0f - v);
        D_L00_0017E660[i][1] = func_002132A8(lo, hi);
        D_L00_0017E660[i][2] = func_002132A8(-0.4f, 0.4f);
        D_L00_0017E660[i][3] = func_002132A8(-0.6f, 0.6f);
        D_L00_0017E660[i][2] += func_002132A8(-0.4f, 0.4f);
        D_L00_0017E660[i][3] += func_002132A8(-0.6f, 0.6f);
        D_L00_0017E660[i][2] += func_002132A8(-0.4f, 0.4f);
        D_L00_0017E660[i][3] += func_002132A8(-0.6f, 0.6f);
    }
}
