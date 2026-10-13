/* Carried from the PAL decompilation's nonmatching/l07_umbris/func_L07_00313298.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L07_00311eb8 -- src/overlays/l07_umbris/vendor_002CE470.c
 * Best so far: BYTES 38/544 (93.0% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Dynamic light slot update (obj+0x280 id, +0x282 owner, +0x284/0x286 fade, +0x288 color, +0x28C): allocates via
 *   p5.c: size and all instructions match; only saved-register naming differs (retail pos=s5, pa=s6, e=s1, color=s
 *   Note: file declares the function with int pa and unsigned color, so the candidate uses aliases/casts.
 */
extern int func_L00_0023E5E0(float *, float, float, int);
extern float func_001FA6C0(int);
extern char D_L07_00180340[];

// Sets or updates a dynamic light slot (id at +0x280) with position, color, radius and fade.
void FUN_L07_00311eb8(float radius, void *moby_, void *obj_, int pa_, void *pos, short b, unsigned int color, int ca) {
    char *moby = moby_;
    char *obj = obj_;
    short pa = pa_;
    float v[4];
    char *e;
    int y;
    if (pos == 0) {
        qcopy(v, moby + 0x10);
    } else {
        qcopy(v, pos);
    }
    if (*(short *)(obj + 0x280) == -1) {
        if (b > 0) {
            if (radius == -1.0f) {
                radius = 15.0f;
            }
            if ((int)color == -1) {
                color = 0x80004080U;
            }
            if (ca == -1) {
                ca = 0;
            }
            *(short *)(obj + 0x280) = func_L00_0023E5E0(v, radius, 0.0f, color);
            *(short *)(obj + 0x284) = 0;
        }
    }
    if (*(short *)(obj + 0x280) != -1) {
        e = D_L07_00180340 + (*(short *)(obj + 0x280) << 5);
        if (*(short *)(obj + 0x282) == pa && (pa == -1 || *(short *)(obj + 0x284) == 0) || *(short *)(obj + 0x282) != pa) {
            if (b != -1) {
                *(short *)(obj + 0x284) = b;
                *(short *)(obj + 0x286) = b;
            }
        } else if (b != *(short *)(obj + 0x282)) {
            y = *(short *)(obj + 0x286) * 7 / 8;
            if (!(y < *(short *)(obj + 0x284))) {
                *(short *)(obj + 0x284) = y;
            }
        }
        if (radius != -1.0f) {
            *(float *)(e + 0x1C) = radius;
        }
        if ((int)color != -1) {
            *(int *)(obj + 0x288) = color;
            *(float *)e = func_001FA6C0(color & 0xFF) * 0.0078125f;
            *(float *)(e + 4) = func_001FA6C0(((int)color >> 8) & 0xFF) * 0.0078125f;
            *(float *)(e + 8) = func_001FA6C0(((int)color >> 16) & 0xFF) * 0.0078125f;
        }
        if (ca != -1) {
            *(int *)(obj + 0x28C) = ca;
        }
        if (pos != 0) {
            qcopy(e + 0x10, pos);
        }
        if (pa != -1) {
            *(short *)(obj + 0x282) = pa;
        }
    }
}
