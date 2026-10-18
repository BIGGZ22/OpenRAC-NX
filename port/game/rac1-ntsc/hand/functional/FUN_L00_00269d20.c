/* Carried from the PAL decompilation's nonmatching/shared/func_L00_0026ABC0.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * functional: high. Read against the US assembly on 2026-10-18: calls (level 0 places), offsets, constants and branches agree; no change. */
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
/* NON_MATCHING FUN_L00_00269d20 -- src/overlays/shared/partupd_0026A130.c
 * Best so far: BYTES 26/1780 (98.5% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Budget run 1: p0.c (= a2.c): BYTES 26/1780.
 *   More free builds after that, none better:
 *   v1/v2: a pos pointer local set before the first test (BYTES 46, 48; the slot unchanged). v5: `goto` out of the
 *   w0: the repack as one nested expression (same bytes as a2). w1: byte casts on the three channels (SIZE 1768).
 *   diag/dw1.c (diagnostic only, banned construct): with three of the four 255 uses given double reference weight,
 *   diag/live0.c (diagnostic only): making the normal's address a function-level pointer used after the join chang
 *   What (1) needs, worked out from reorg (fill_slots_from_thread): for the fall-through thread a candidate must n
 *   What I could not find: which pseudo that ends in $s0 was live into the else block in retail's flow information
 */
extern s32 func_L00_001EFFF0_70DB0(void *, void *, s32, void *, void *) __asm__("func_L00_001EFC70");
extern void func_L00_0025C2B0(void *, void *, void *, float);
extern void func_L00_001FF500(void *, void *, float);
void func_001F9BD8_26bca8(void *, void *, void *) __asm__("func_001F9A10");
extern float func_001FA6C0(int);
int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_001FA050(void *, void *);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_001FF598(void *, void *, float);
int func_001F9938_26bca8(void *) __asm__("func_001F9770");
extern void func_L00_00267A08(void *);
extern f32 D_L00_00173E80[];
extern float D_0015ED60 MACRO_ADDR;
typedef int u128_6AB_c269d20 __attribute__((mode(TI)));

/* Particle type 6 update: moves the particle, clamps its velocity and colour, then kills it when it leaves its bounds. */
void FUN_L00_00269d20(unsigned char *m) {
    unsigned char *t;
    unsigned char *e;
    float tmp[4];
    float o[4];
    float v2[4];
    float v3[4];
    float v4[16];
    float k;
    float f22, f23, f24;
    int fl;
    int bb;
    int r4, r5, r6, r7;

    e = m + 0x20;
    t = *(unsigned char **)(e + 0xC);
    tmp[0] = *(float *)(m + 0x10) + *(float *)(m + 0x20);
    tmp[1] = *(float *)(m + 0x14) + *(float *)(e + 0x4);
    tmp[2] = *(float *)(m + 0x18) + *(float *)(e + 0x8);

    if ((*(int *)(t + 0x70) & 1) && func_L00_001EFFF0_70DB0(m + 0x10, tmp, 4, m, 0)) {
        v2[0] = *(float *)(m + 0x20);
        v2[1] = *(float *)(e + 0x4);
        v2[2] = *(float *)(e + 0x8);
        func_L00_0025C2B0(o, v2, D_L00_00173E80, *(float *)(t + 0x3C));
        *(float *)(m + 0x20) = o[0];
        *(float *)(e + 0x4) = o[1];
        *(float *)(e + 0x8) = o[2];
        func_L00_001FF500(v3, D_L00_00173E80, 0.055f);
        func_001F9BD8_26bca8(m + 0x10, (char *)D_L00_00173E80 - 0x20, v3);
    } else {
        *(u128_6AB_c269d20 *)(m + 0x10) = *(u128_6AB_c269d20 *)tmp;
    }

    fl = *(int *)(t + 0x70);
    if (fl & 2) {
        f24 = func_001FA6C0((m[4] << 8) | e[0x18]) / 65535.0f;
        f23 = (float)func_001FA898_r((float)((*(int *)(m + 4) & 0xFF00) | e[0x19])) / 65535.0f;
        f22 = (float)func_001FA898_r((float)((int)(((unsigned int)*(int *)(m + 4) >> 8) & 0xFF00) | e[0x1A])) / 65535.0f;

        if (e[0x1B] & 0x10) {
            f24 = f24 - *(float *)(t + 0x4C) * D_0015ED60;
            k = D_0015ED60;
        } else {
            f24 = f24 + *(float *)(t + 0x4C) * D_0015ED60;
            k = D_0015ED60;
        }
        if (e[0x1B] & 0x20) {
            f23 = f23 - *(float *)(t + 0x50) * k;
        } else {
            f23 = f23 + *(float *)(t + 0x50) * k;
        }
        if (e[0x1B] & 0x40) {
            f22 = f22 - *(float *)(t + 0x54) * D_0015ED60;
        } else {
            f22 = f22 + *(float *)(t + 0x54) * D_0015ED60;
        }

        if (*(float *)(t + 0x64) < f24) {
            f24 = *(float *)(t + 0x64);
            if (*(int *)(t + 0x70) & 0x100) e[0x1B] ^= 0x10;
        }
        if (f24 < *(float *)(t + 0x58)) {
            f24 = *(float *)(t + 0x58);
            if (*(int *)(t + 0x70) & 0x10) e[0x1B] ^= 0x10;
        }
        if (*(float *)(t + 0x68) < f23) {
            f23 = *(float *)(t + 0x68);
            if (*(int *)(t + 0x70) & 0x200) e[0x1B] ^= 0x20;
        }
        if (f23 < *(float *)(t + 0x5C)) {
            f23 = *(float *)(t + 0x5C);
            if (*(int *)(t + 0x70) & 0x20) e[0x1B] ^= 0x20;
        }
        if (*(float *)(t + 0x6C) < f22) {
            f22 = *(float *)(t + 0x6C);
            if (*(int *)(t + 0x70) & 0x400) e[0x1B] ^= 0x40;
        }
        if (f22 < *(float *)(t + 0x60)) {
            f22 = *(float *)(t + 0x60);
            if (*(int *)(t + 0x70) & 0x40) e[0x1B] ^= 0x40;
        }

        e[0x18] = func_001FA898_r(f24 * 65535.0f);
        e[0x19] = func_001FA898_r(f23 * 65535.0f);
        e[0x1A] = func_001FA898_r(f22 * 65535.0f);
        r4 = func_001FA898_r(f24 * 255.0f);
        r5 = func_001FA898_r(f23 * 255.0f);
        r6 = func_001FA898_r(f22 * 255.0f);
        r7 = func_001FA898_r(*(float *)(e + 0x10) * 255.0f);
        *(int *)(m + 4) = (r4 & 0xFF) | ((r5 & 0xFF) << 8) | ((r6 & 0xFF) << 16) | (r7 << 24);
        goto dtest;
    }
    if (!(fl & 4)) goto b0b0;
    r7 = func_001FA898_r(*(float *)(e + 0x10) * 255.0f);
    *(int *)(m + 4) = (*(int *)(m + 4) & 0xFFFFFF) | (r7 << 24);
dtest:
    if (*(int *)(t + 0x70) & 4) {
        k = D_0015ED60;
        if (e[0x1B] & 0x80) {
            *(float *)(e + 0x10) = *(float *)(e + 0x10) - *(float *)(t + 0x38) * k;
        } else {
            *(float *)(e + 0x10) = *(float *)(e + 0x10) + *(float *)(t + 0x38) * k;
        }
        if (*(float *)(e + 0x10) > *(float *)(t + 0x48)) {
            *(float *)(e + 0x10) = *(float *)(t + 0x48);
            if (*(int *)(t + 0x70) & 0x800) e[0x1B] ^= 0x80;
        }
        if (*(float *)(e + 0x10) < *(float *)(t + 0x44)) {
            *(float *)(e + 0x10) = *(float *)(t + 0x44);
            if (*(int *)(t + 0x70) & 0x80) e[0x1B] ^= 0x80;
        }
    }
b0b0:
    m[8] = func_001FA898_r(*(float *)(e + 0x14) * 255.0f);

    if (*(int *)(t + 0x70) & 8) {
        tmp[0] = *(float *)(e + 0x0);
        tmp[1] = *(float *)(e + 0x4);
        tmp[2] = *(float *)(e + 0x8);
        if (*(int *)(t + 0x70) & 0x8000) {
            func_001FA050(v4, *(char **)(t + 0x7C) + 0x40);
            func_001F9D20(o, t, v4);
        } else {
            *(u128_6AB_c269d20 *)o = *(u128_6AB_c269d20 *)t;
        }
        func_001F9BD8_26bca8(tmp, tmp, o);
        func_L00_001FF598(tmp, tmp, *(float *)(t + 0x40));
        *(float *)(e + 0x0) = tmp[0];
        *(float *)(e + 0x4) = tmp[1];
        *(float *)(e + 0x8) = tmp[2];
    }

    *(float *)(m + 0xC) = *(float *)(m + 0xC) + *(float *)(t + 0x30) * D_0015ED60;
    if (*(float *)(m + 0xC) < *(float *)(t + 0x74)) *(float *)(m + 0xC) = *(float *)(t + 0x74);
    if (*(float *)(m + 0xC) > *(float *)(t + 0x78)) *(float *)(m + 0xC) = *(float *)(t + 0x78);

    *(float *)(e + 0x14) = *(float *)(e + 0x14) + *(float *)(t + 0x34) * D_0015ED60;
    if (*(float *)(e + 0x10) <= 0.0f || func_001F9938_26bca8(m + 0xA) || *(float *)(m + 0x10) < *(float *)(t + 0x10) || *(float *)(m + 0x14) < *(float *)(t + 0x14) || *(float *)(m + 0x18) < *(float *)(t + 0x18) || *(float *)(t + 0x20) < *(float *)(m + 0x10) || *(float *)(t + 0x24) < *(float *)(m + 0x14) || *(float *)(t + 0x28) < *(float *)(m + 0x18)) {
        func_L00_00267A08(m);
    }
}
