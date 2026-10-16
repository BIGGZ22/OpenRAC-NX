/* Carried from the PAL decompilation's nonmatching/shared/func_L14_00303270.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L14_00301de8 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: BYTES 16/444 (96.4% of the bytes match), checked 2026-10-16.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   The pvars as one struct of vec4 arrays (keys, pos, vel, cur, ring[4][4], spin[3][4]): right size first try.
 *   The copy loop needs its own counter (k), else it lands in a saved register.
 *   Left: the ring/spin loop. Retail forms d + j*16, copies it, then adds 0x460 and 0x560 (two givs, CSE copy).
 *   Ours forms j*16 + 0x460 and adds d. Same for ring[i][j], row pointers, d->ring[i] + j, Mtx rows.
 *   A cast column base gets the order but not the copy (13/444). Flat vec4 indexing grows to 452.
 */
typedef struct { float x, y, z, w; } Vec4_303270_c301de8;
typedef struct {
    char pad0[0x12];
    short count;
    int color;
    unsigned char order[4];
    char pad1C[4];
    Vec4_303270_c301de8 keys[4];
    Vec4_303270_c301de8 pos[32];
    Vec4_303270_c301de8 vel[28];
    Vec4_303270_c301de8 cur[4];
    Vec4_303270_c301de8 ring[4][4];
    Vec4_303270_c301de8 spin[3][4];
    int start;
} Chain_303270_c301de8;
extern void func_001F9A10(void *, void *, void *);
extern int func_001FA6E0(int, int, float);
extern float func_001FA888_303270(int) __asm__("func_001FA6C0");
extern int D_L14_001620C4 SDATA(D_L14_001620C4);
extern int D_L14_00162198_i SDATA(D_L14_001620C8);
extern int D_L14_001620F0 SDATA(D_L14_001620F0);
extern int D_L14_001620F4 SDATA(D_L14_001620F4);
extern int D_L14_001620F8 SDATA(D_L14_001620F8);



/* while the count runs, moves 23 points by their velocities, turns three rows of four by their
   spins and copies four keys in their order; then tweens the color over the count's two stages */
void FUN_L14_00301de8(char *moby) {
    Chain_303270_c301de8 *d = *(Chain_303270_c301de8 **)(moby + 0x78);
    int i, j, k;

    if (d->count != 0) {
        d->count--;
        for (i = 0; i < 23; i++) {
            func_001F9A10(&d->pos[i], &d->pos[i], &d->vel[i]);
        }
        for (j = 0; j < 4.0f; j++) {
            for (i = 0; i < 3; i++) {
                func_001F9A10(&d->ring[i][j], &d->ring[i][j], &d->spin[i][j]);
            }
        }
        for (k = 0; k < 4; k++) {
            qcopy(&d->cur[k], &d->keys[d->order[k]]);
        }
    }
    if (D_L14_001620C4 < d->count) {
        d->color = func_001FA6E0(D_L14_001620F4, D_L14_001620F0,
                                func_001FA888_303270(d->count - D_L14_001620C4) /
                                func_001FA888_303270(d->start - D_L14_001620C4));
    } else if (D_L14_00162198_i < d->count) {
        d->color = func_001FA6E0(D_L14_001620F8, D_L14_001620F4,
                                func_001FA888_303270(d->count - D_L14_00162198_i) /
                                func_001FA888_303270(D_L14_001620C4 - D_L14_00162198_i));
    } else {
        d->color = 0;
    }
}
