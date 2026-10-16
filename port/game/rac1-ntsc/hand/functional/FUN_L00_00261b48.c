/* Carried from the PAL decompilation's src/overlays/shared/mobyutil_00261B00.c (matched C there) (games/rac1) through the version
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
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c261b48 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c261b48 q;
    f32 f[4];
    s32 i[4];
} Vec4_c261b48;
typedef struct {
    s32 count;
    u8 pad[12];
    Vec4_c261b48 points[1];
} SegmentPath_2629E0_c261b48;
extern SegmentPath_2629E0_c261b48 *D_L00_001B0830_2629E0[] __asm__("D_L00_001B04B0");
extern void func_001F9A28(void *, void *, void *);
extern float func_001F9AF0(void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F99F8(void *);
extern void func_001F9CF8(void *, void *, void *);
extern void func_001F9A40(void *, void *, void *, float);



int FUN_L00_00261b48(int idx, void *a, void *b, void *c) {
    float v[4], w[4], z[4], t[4], u[4];
    float best;
    float zero;
    int found = 0;
    int i = 1;
    int offset;
    float *prev;
    float *cur;
    func_001F9A28(v, b, a);
    v[2] = 0;
    func_L00_001FF500(v, v, 1.0f / func_001F9AF0(v));
    func_001F99F8(w);
    w[0] = v[1];
    w[1] = -v[0];
    func_001F99F8(z);
    z[2] = 1.0f;
    best = 1.0f;
    func_001F9A28(t, (char *)D_L00_001B0830_2629E0[idx] + 0x10, a);
    func_001F9CF8(t, t, v);
    prev = t;
    cur = u;
    if (i < D_L00_001B0830_2629E0[idx]->count) {
        offset = 0x20;
        zero = 0.0f;
        do {
            func_001F9A28(cur, (char *)D_L00_001B0830_2629E0[idx] + offset, a);
            func_001F9CF8(cur, cur, v);
            if (t[3] != zero || u[3] != zero) {
                if (u[1] * t[1] < 0.0f) {
                    float x = (t[0] - u[0]) / (t[1] - u[1]) * -u[1] + u[0];
                    if (x > 0.0f && x < best) {
                        best = x;
                        found = 1;
                        qcopy(prev, cur);
                        goto next;
                    }
                }
            }
            qcopy(prev, cur);
        next:
            i++;
            offset += 0x10;
        } while (i < D_L00_001B0830_2629E0[idx]->count);
    }
    func_001F9A40(c, a, b, best);
    return found;
}
