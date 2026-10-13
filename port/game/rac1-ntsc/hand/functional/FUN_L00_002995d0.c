/* Carried from the PAL decompilation's nonmatching/shared/func_L00_0029A940.c (games/rac1/pal) through the version
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
/* NON_MATCHING FUN_L00_002995d0 -- src/overlays/shared/tieproc_00299108.c
 * Best so far: SIZE ours 496 / retail 504, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   FUN_L00_002995d0 (StartPssMovie): sets loader globals, then uploads two runs of image strips (0xD/0xE 128x128
 *   Best candidate p6.c/p3.c (SIZE 496 vs 504): the loop counters, movn selection and copy of D_L00_0015F40C into 
 *   Ours puts `sw D_L00_00161058` in the `beq n1,0` delay slot (check_macro_slots turns it gp-relative, 1 insn ins
 */
extern char D_0013E533[];
extern void func_00118A80(int);
extern void func_0022DCD0(void);
extern void func_00215EE8(void);
extern void func_001F4A58(int);
extern void func_L00_002039A0(void);
extern int func_00122330(void *, short, short, short, short, short, short, short);
extern int func_00122658(void *, void *);
extern int func_00120558(int, unsigned short);
extern int func_001E93B0();
extern void func_001F97B0(void);
extern int D_L00_00161058 MACRO_ADDR;
extern int D_L00_00161050 MACRO_ADDR;
extern int D_L00_0016104C MACRO_ADDR;
extern int D_L00_00161048 MACRO_ADDR;
extern int D_L00_00161114_m __asm__("D_L00_00161054") MACRO_ADDR;
extern int D_0015ED80 MACRO_ADDR;
extern int D_L00_0015F5C4 MACRO_ADDR;
extern int D_0015EE88;
extern short D_0015EF78_g __asm__("D_0015EE78");
extern short D_L00_0015F6BC_g __asm__("D_L00_0015F5D8");
extern int D_L00_0015F40C;
extern int D_L00_00173E04;
extern char D_L00_001E9480[];

/* Start a movie: set up the loader globals and upload the two image strips. */
void FUN_L00_002995d0(int arg0, int arg1, int arg2) {
    int buf[24];
    int n1, n2, s16, s17, i, t;
    D_0013E533[0x88] |= 8;
    func_00118A80(0);
    func_0022DCD0();
    func_00215EE8();
    D_L00_00161048 = arg0;
    D_L00_00161050 = arg2;
    D_L00_0016104C = arg1;
    *(int *)&D_L00_0015F6BC_g = 1;
    func_001F4A58(4);
    func_L00_002039A0();
    t = D_L00_0015F40C;
    D_L00_0015F5C4 = 1;
    D_L00_00161114_m = D_L00_00173E04;
    D_L00_00161058 = t;
    n1 = 0xD;
    n2 = 0x11;
    if (D_0015ED80 != 0) {
        n1 = 0xE;
        n2 = 0xE;
    }
    s17 = t;
    s16 = D_0015EE88;
    for (i = 0; i < n1; i++) {
        func_00122330(buf, s16 >> 8, 2, 1, 0, 0, 0x80, 0x80);
        s16 += 0x10000;
        func_00118A80(0);
        func_00122658(buf, (void *)s17);
        s17 += 0xC000;
        func_00120558(0, 0);
    }
    s16 = *(int *)&D_0015EF78_g;
    for (i = 0; i < n2; i++) {
        func_00122330(buf, s16 >> 8, 1, 0, 0, 0, 0x40, 0x40);
        s16 += 0x4000;
        func_00118A80(0);
        func_00122658(buf, (void *)s17);
        s17 += 0x4000;
        func_00120558(0, 0);
    }
    if (s16 > 0x400000) {
        func_001E93B0(D_L00_001E9480, s16 - 0x400000);
        func_001F97B0();
    }
}
