/* Carried from the PAL decompilation's nonmatching/shared/func_L00_0029AFB8.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_L00_0029AFB8 (near) was paired by place; read it against NTSC-U before trusting it. */
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
extern s32 func_002168A8(s32);
extern char D_0013A4E0[];

/* NON_MATCHING FUN_L00_00299c48 -- src/overlays/shared/tieproc_00299108.c
 * Best so far: SIZE ours 748 / retail 756, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Cannot land as written (#define in a candidate): rewrite that in plain C first.
 * What the last attempts found:
 *   Per-frame state machine for the level intro/stream swap: fades D_L00_0015F3FC, steps D_L00_0015F620 state 0 (w
 *   Structure and all gp/lui forms match except SIZE 748 vs 756: retail never fills the delay slots of the branche
 *   Would unblock: knowing why retail's scheduler does not thread the tail load into those slots (probably a diffe
 */
extern void func_L00_00299C00(void);
extern void func_002337B0(int);
extern int func_00217628_a(int *, int, int) __asm__("func_00216788");
extern void func_0022CA50(void);
extern int D_L00_0015F3FC MACRO_ADDR;
extern int D_L00_0015F702_m __asm__("D_L00_0015F622") MACRO_ADDR;
#define D_L00_0015F622 (*(short *)&D_L00_0015F702_m)
extern int D_L00_0015F704_m __asm__("D_L00_0015F624") MACRO_ADDR;
#define D_L00_0015F624 (*(short *)&D_L00_0015F704_m)
extern int D_L00_0015F706_m __asm__("D_L00_0015F626") MACRO_ADDR;
#define D_L00_0015F626 (*(short *)&D_L00_0015F706_m)
extern int D_L00_0015F700_mm __asm__("D_L00_0015F620") MACRO_ADDR;
extern int D_L00_00173C00[];
extern int *D_L00_0015F628 MACRO_ADDR;
extern int *D_L00_0015F62C MACRO_ADDR;
extern int D_0015ED80 MACRO_ADDR;
extern char D_00137B80[];
extern unsigned char D_0014161B[] NOT_SDA;

void FUN_L00_00299c48(void) {
    int s0, t, i, r;
    int *src, *dst;
    float f;
    func_002168A8(0);
    f = *(float *)&D_L00_0015F3FC - 0.05f;
    *(float *)&D_L00_0015F3FC = f;
    if (f < 0.0f) {
        *(float *)&D_L00_0015F3FC = 0.0f;
    }
    s0 = D_0013A4E0[0x2CF2] == 0x41;
    if ((*(short *)&D_L00_0015F700_mm) == 0) {
        if (D_L00_0015F626 >= D_L00_00173C00[D_L00_0015F622] - 0x24) {
            if (*(short *)(D_0014161B + 0x100BD) != 0) {
                func_002168A8(1);
            }
            if ((!s0 && D_L00_0015F622 == 0x29) || D_L00_0015F622 >= 0x2D) {
                func_L00_00299C00();
                return;
            }
            D_L00_0015F624 = -1;
            (*(short *)&D_L00_0015F700_mm) = 1;
        }
    } else if ((*(short *)&D_L00_0015F700_mm) == 1) {
        func_002337B0(1);
        t = D_L00_0015F624;
        if (t < 0x20) {
            int off = t * 4;
            src = (int *)((char *)D_L00_0015F62C + off);
            dst = (int *)((char *)D_L00_0015F628 + off);
            for (i = 0; i <= 0x37FFF; i += 0x20) {
                *dst = *src;
                dst += 0x20;
                src += 0x20;
            }
        } else if (t >= 0x24) {
            if (!s0) {
                if (D_L00_0015F622 == 8) {
                    D_L00_0015F622 = 0x11;
                    D_L00_0015F626 = 0x1590;
                }
            } else {
                if (D_L00_0015F622 == 0x11) {
                    D_L00_0015F622 = 0x1B;
                    D_L00_0015F626 = 0x2238;
                }
                if (D_L00_0015F622 == 0x27) {
                    D_L00_0015F622 = 0x29;
                    D_L00_0015F626 = 0x32B4;
                }
            }
            (*(short *)&D_L00_0015F700_mm) = 0;
            D_L00_0015F624 = -1;
            D_L00_0015F622 = D_L00_0015F622 + 1;
            r = D_L00_0015F622 % 20;
            if (D_0015ED80 != 0) {
                func_00217628_a(D_L00_0015F62C, *(int *)(D_00137B80 + 0x1748 + r * 8), *(int *)(D_00137B80 + 0x174C + r * 8));
            } else {
                func_00217628_a(D_L00_0015F62C, *(int *)(D_00137B80 + 0x16A8 + r * 8), *(int *)(D_00137B80 + 0x16AC + r * 8));
            }
        }
    }
    D_L00_0015F624 = D_L00_0015F624 + 1;
    D_L00_0015F626 = D_L00_0015F626 + 1;
    if (*(int *)(D_0013A4E0 + 0x2604) & 0x800) {
        func_L00_00299C00();
    }
    func_0022CA50();
}
