/* Carried from the PAL decompilation's nonmatching/shared/func_L00_0029ADD8.c (games/rac1) through the version
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
extern s32 func_002168A8(s32);

/* NON_MATCHING FUN_L00_00299a68 -- src/overlays/shared/tieproc_00299108.c
 * Best so far: SIZE ours 400 / retail 404, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   EnterSlideshowMode: starts the slideshow (func_002168A8 x2, rebases the level pointers D_L00_001611CC/F708/F70
 *   Best try p7.c (400/404 bytes, SIZE mismatch): the code and the load/store order match retail, but the compiler
 *   Unblock: a declaration form for D_L00_0015F620+{2,8,0xC} that makes the store length 8 to the compiler (not sl
 */
extern void func_001F4A58(int);
extern void func_001F97E8(void *, int, int);
extern int func_00217628_w(int, int, int) __asm__("func_00216788");
extern int D_L00_0015F5D8 MACRO_ADDR;
extern int D_L00_0015F5C4 MACRO_ADDR;
extern short D_L00_0015F4FC_g __asm__("D_L00_0015F3FC");
extern int D_L00_0015F620[];
extern int D_L00_00173E00[];
extern int D_L00_001611CC MACRO_ADDR;
extern int D_L00_0015F62C MACRO_ADDR;
extern int D_L00_0015F628 MACRO_ADDR;
extern short D_L00_0015F622[2] MACRO_ADDR;
extern int D_0015ED80 MACRO_ADDR;
typedef struct { int b; int a; } LL9_c299a68;
typedef struct { char pad[0x16A8]; LL9_c299a68 normal[20]; LL9_c299a68 alt[20]; } G9_c299a68;
extern G9_c299a68 D_00137C80_h __asm__("D_00137B80");

/* starts the slideshow: resets state, rebases the level data pointers, and starts the first two streams */
void FUN_L00_00299a68(void) {
    int t;
    func_002168A8(0);
    func_001F4A58(8);
    D_L00_0015F5D8 = 1;
    D_L00_0015F5C4 = 7;
    *(float *)&D_L00_0015F4FC_g = 1.0f;
    func_001F97E8(D_L00_0015F620, 0, 16);
    t = D_L00_001611CC - 0xE0000;
    D_L00_0015F628 = D_L00_00173E00[1] + t;
    D_L00_0015F62C = D_L00_00173E00[2] + t;
    D_L00_001611CC = t;
    if (D_0015ED80 != 0) {
        func_00217628_w(D_L00_0015F628, D_00137C80_h.alt[D_L00_0015F622[0]].a, D_00137C80_h.alt[D_L00_0015F622[0]].b);
    } else {
        func_00217628_w(D_L00_0015F628, D_00137C80_h.normal[D_L00_0015F622[0]].a, D_00137C80_h.normal[D_L00_0015F622[0]].b);
    }
    func_002168A8(1);
    D_L00_0015F622[0]++;
    if (D_0015ED80 != 0) {
        func_00217628_w(D_L00_0015F62C, D_00137C80_h.alt[D_L00_0015F622[0]].a, D_00137C80_h.alt[D_L00_0015F622[0]].b);
    } else {
        func_00217628_w(D_L00_0015F62C, D_00137C80_h.normal[D_L00_0015F622[0]].a, D_00137C80_h.normal[D_L00_0015F622[0]].b);
    }
}
