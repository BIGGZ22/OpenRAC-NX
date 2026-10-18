/* US check 2026-10-17: corrected, high confidence (slide table, three indices and one callee were PAL values; see below). */
/* Carried from the PAL decompilation's src/overlays/shared/tieproc_00299108.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_L00_0029AFB8 (near) was paired by place; read it against NTSC-U before trusting it.
 * Checked against the US assembly (2026-10-17): slide table D_L00_00173BF0 (PAL 173C00), quit/skip indices 0x27/0x25/0x2B
 * (PAL 0x29/0x27/0x2D), and the state-1 call is FUN_L00_002a09d8(1) (PAL func_002337B0). Confidence: high. */
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
extern char D_0013A4E0[];
extern unsigned char D_0014161B[] NOT_SDA;
extern s32 func_002168A8(s32);
extern int D_0015ED80 MACRO_ADDR;
extern int func_00216788(int, int, int);
extern int D_L00_0015F620[4] MACRO_ADDR;
typedef struct { int a; int b; } LL9_c299c48;
typedef struct { char pad[0x16A8]; LL9_c299c48 normal[20]; LL9_c299c48 alt[20]; } G9_c299c48;
extern G9_c299c48 D_00137B80;
extern int D_L00_0015F3FC MACRO_ADDR;
extern void func_L00_00299C00(void);
extern void func_0022CA50(void);
extern int D_L00_00173BF0[];
extern void FUN_L00_002a09d8(int);
#define IST (((short *)D_L00_0015F620)[0])
#define IIDX (((short *)D_L00_0015F620)[1])
#define ICNT (((short *)D_L00_0015F620)[2])
#define ITMR (((short *)D_L00_0015F620)[3])



void FUN_L00_00299c48(void) {
    int s0, t, i, r, one;
    int *src, *dst;
    float f;
    func_002168A8(0);
    f = *(float *)&D_L00_0015F3FC - 0.05f;
    *(float *)&D_L00_0015F3FC = f;
    if (f < 0.0f) {
        *(float *)&D_L00_0015F3FC = 0.0f;
    }
    s0 = *(unsigned char *)(D_0013A4E0 + 0x2CF2) == 0x41;
    one = 1;
    if (IST == 0) {
        if (ITMR >= D_L00_00173BF0[IIDX] - 0x24) {
            if (*(short *)(D_0014161B + 0x100BD) != 0) {
                func_002168A8(1);
            }
            if (!s0 && IIDX == 0x27) {
            quit:
                func_L00_00299C00();
                return;
            }
            if (IIDX >= 0x2B) goto quit;
            ICNT = -1;
            IST = 1;
        }
    } else if (IST == one) {
        FUN_L00_002a09d8(1);
        t = ICNT;
        if (t < 0x20) {
            src = (int *)D_L00_0015F620[3];
            dst = (int *)D_L00_0015F620[2];
            src += t;
            dst += t;
            for (i = 0; i <= 0x37FFF; i += 0x20) {
                *dst = *src;
                dst += 0x20;
                src += 0x20;
            }
        } else if (t >= 0x24) {
            if (!s0) {
                if (IIDX == 8) {
                    IIDX = 0x11;
                    ITMR = 0x1590;
                }
            } else {
                if (IIDX == 0x11) {
                    IIDX = 0x1B;
                    ITMR = 0x2238;
                }
                if (IIDX == 0x25) {
                    IIDX = 0x27;
                    ITMR = 0x32B4;
                }
            }
            IST = 0;
            ICNT = -1;
            IIDX = IIDX + 1;
            r = IIDX % 20;
            if (D_0015ED80 != 0) {
                func_00216788(D_L00_0015F620[3], D_00137B80.alt[r].a, D_00137B80.alt[r].b);
            } else {
                func_00216788(D_L00_0015F620[3], D_00137B80.normal[r].a, D_00137B80.normal[r].b);
            }
        }
    }
    ICNT = ICNT + 1;
    ITMR = ITMR + 1;
    if (*(int *)(D_0013A4E0 + 0x2604) & 0x800) {
        func_L00_00299C00();
    }
    func_0022CA50();
}
