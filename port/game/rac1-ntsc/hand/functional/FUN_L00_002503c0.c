/* Carried from the PAL decompilation's nonmatching/functional/func_L00_002513F8.c (games/rac1) through the version
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
/* functional: high. Checked against the US assembly (build/overlay-asm/level_00/split/f002503C0, one 112-byte
 * function): ld of obj+0x38, byte fields b0/b1/b2, bgtz on b2 (so b2 == 0 copies e0's quad), vitof12 for t = b2/256,
 * vsubx.w for 1 - t, vmulaw/vmaddx for the xyz blend, w left as e0's. No difference found; VU rounding (ACC then
 * madd) written as separate float operations; not run.
 * Reads bytes 0-2 of the 8 at obj+0x38 (b0, b1, b2; retail loads all 8 and shifts); table D_L00_0017FEC0 has 64-byte entries whose first quad is a colour/vector. If b2 == 0 copies entry b0's quad to out; else out.xyz = e0.xyz*(1-t) + e1.xyz*t with t = b2/256 (e0 = entry b0, e1 = entry b1) and out.w = e0.w. */
extern char D_L00_0017FFC0_513F8[] __asm__("D_L00_0017FEC0");



void FUN_L00_002503c0(char *obj, float *out) {
    unsigned char *v = (unsigned char *)(obj + 0x38);
    unsigned int b0 = v[0];
    unsigned int b1 = v[1];
    unsigned int b2 = v[2];
    float *e0 = (float *)(D_L00_0017FFC0_513F8 + (b0 << 6));

    if ((int)b2 <= 0) {
        out[0] = e0[0];
        out[1] = e0[1];
        out[2] = e0[2];
        out[3] = e0[3];
    } else {
        float *e1 = (float *)(D_L00_0017FFC0_513F8 + (b1 << 6));
        float t = (float)(int)(b2 << 4) * (1.0f / 4096.0f);
        float w = 1.0f - t;
        float x = e0[0] * w;
        float y = e0[1] * w;
        float z = e0[2] * w;
        float x2 = e1[0] * t;
        float y2 = e1[1] * t;
        float z2 = e1[2] * t;
        out[0] = x + x2;
        out[1] = y + y2;
        out[2] = z + z2;
        out[3] = e0[3];
    }
}
