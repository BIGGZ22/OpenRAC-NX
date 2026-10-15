/* Carried from the PAL decompilation's nonmatching/l12_hoven/func_L12_003078C8.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L12_003064e8 -- src/overlays/l12_hoven/vendor_002EDAA0.c
 * Best so far: BYTES 22/832 (97.4% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   What it does: loops over 18 slots (D_L12_001FB688, 256-byte objects at D_L12_0015FFD8); slot 17 is the moby it
 *   Still different from retail (p5):
 *   - 0x500 branch: retail sets the int arg ($6 = 0) before loading the float constants for func_L00_0025BC98; our
 *   - 0x4FD branch: retail computes data + 16i first, then adds 0x454 (addu $a1,$s2,$s5 / addiu); ours does the re
 *   - Prologue/saved-register and stack-slot layout differs slightly from retail (retail keeps the counter at 0($s
 *   Would unblock: a way to keep the loop counter spilled while getting the 0x4FD address order, or a form that ma
 *   HQ s20 round (5 runs, p7-p11): best p8.c, now the same size as retail (832) and 22 bytes off. The 0x500 call s
 *   Unblock: a way to raise data's priority over the constant 17 without changing the code, or a form for the 0x4F
 */
extern void func_0020C828(void *);
extern float func_001FA580(float, float);
extern float func_L00_0025BC98(void *, float, void *, int, float, float, float);
extern void func_L00_00250DF8(void *);
extern int D_L12_001FB688[];
extern int D_L12_00160058_m __asm__("D_L12_0015FFD8") MACRO_ADDR;
extern float D_L12_00161F40 MACRO_ADDR;
extern float D_L12_00161F50 MACRO_ADDR;
extern float D_L12_00161F60 MACRO_ADDR;
extern float D_L12_00161F70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;

// Level 12 vendor: updates each of the 18 slot objects for one frame.
void FUN_L12_003064e8(char *moby) {
    int i;
    int off;
    char *data = *(char **)(moby + 0x78);
    for (i = 0, off = 0; i < 18; i++, off += 4) {
        int s = D_L12_001FB688[i];
        char *obj;
        float f1;
        if (s == 0) {
            continue;
        }
        if (i == 17) {
            obj = moby;
        } else {
            if (s == -1) {
                continue;
            }
            obj = (char *)(D_L12_00160058_m + (s << 8));
        }
        if (i == 17) {
            f1 = D_L12_00161F40;
            if (f1 == -1.0f) {
                qcopy((char *)&D_L12_00161F40, obj + 0x10);
                qcopy((char *)&D_L12_00161F50, obj + 0x40);
            }
            qcopy((char *)&D_L12_00161F60, obj + 0x10);
            qcopy((char *)&D_L12_00161F70, obj + 0x40);
            qcopy(obj + 0x10, (char *)&D_L12_00161F40);
            qcopy(obj + 0x40, (char *)&D_L12_00161F50);
            f1 = *(float *)(obj + 0x18);
        } else {
            f1 = *(float *)(obj + 0x18);
        }
        if (f1 < 20.0f) {
            if (i == 17) {
                return;
            }
            func_0020C828(obj);
            D_L12_001FB688[i] = -1;
        }
        f1 = *(float *)(data + 0x338 + i * 16) - *(float *)(data + 0x570 + off);
        *(float *)(data + 0x338 + i * 16) = f1;
        *(float *)(obj + 0x18) = *(float *)(obj + 0x18) + f1;
        if (*(short *)(obj + 0xA6) == 0x500) {
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), D_0015ED6C * 0.34906584f);
            func_L00_0025BC98(obj + 0x44, 1.5707964f, data + 0x454 + i * 16, 0, 0.00200000009f, 0.3f, 0.0f);
        } else if (*(short *)(obj + 0xA6) == 0x4FD) {
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), *(float *)(data + 0x450 + i * 16));
            func_L00_0025BC98(obj + 0x44, -1.5707964f, data + 0x454 + i * 16, 0, *(float *)(data + 0x45C + i * 16), 0.3f, 0.0f);
        } else if (*(short *)(obj + 0xA6) == 0x4FA) {
            float t;
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), D_0015ED6C * -0.87266463f);
            t = func_001FA580(*(float *)(obj + 0x44), *(float *)(data + 0x454 + i * 16));
            *(float *)(obj + 0x14) = *(float *)(obj + 0x14) + D_0015ED6C * -4.0f;
            *(float *)(obj + 0x44) = t;
        } else {
            *(float *)(obj + 0x14) = *(float *)(obj + 0x14) + D_0015ED6C * -4.0f;
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), *(float *)(data + 0x450 + i * 16));
            *(float *)(obj + 0x44) = func_001FA580(*(float *)(obj + 0x44), *(float *)(data + 0x454 + i * 16));
        }
        if (i == 17) {
            qcopy((char *)&D_L12_00161F40, obj + 0x10);
            qcopy((char *)&D_L12_00161F50, obj + 0x40);
        }
        func_L00_00250DF8(obj);
    }
}
