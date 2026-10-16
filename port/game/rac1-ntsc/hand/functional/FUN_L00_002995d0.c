/* Carried from the PAL decompilation's src/overlays/shared/tieproc_00299108.c (matched C there) (games/rac1) through the version
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
extern int D_L00_0015F5C4 MACRO_ADDR;
extern int D_L00_0015F5D8 MACRO_ADDR;
extern void func_001F4A58(int);
extern int D_0015ED80 MACRO_ADDR;
extern char D_0013E550[];
extern void func_00118A80(int);
extern void func_L00_002039A0(void);
extern int func_00122330(void *, short, short, short, short, short, short, short);
extern int func_00122658(void *, void *);
extern int func_00120558(int, unsigned short);
extern int func_001E93B0(char *, int);
extern void func_001F97B0(void);
typedef struct {
    int sector;
    int size;
    int unk8;
    int buf0;
    int buf1;
} MovieLoad_c2995d0;
extern MovieLoad_c2995d0 D_L00_00161108_m __asm__("D_L00_00161048") MACRO_ADDR;
extern int D_0015EF78_29A940[6] __asm__("D_0015EE78") MACRO_ADDR;
extern int D_L00_0015F40C MACRO_ADDR;
extern int D_L00_00173E04;
extern char D_L00_001E9480[];



/* sets up a movie load, then uploads image data from the level buffer to GS memory in two runs of strips */
void FUN_L00_002995d0(int sector, int size, int arg2) {
    int image[24];
    int big, small, gs_addr, src, i, j;
    D_0013E550[0x6B] |= 8;
    func_00118A80(0);
    func_0022DCD0();
    func_00215EE8();
    D_L00_00161108_m.sector = sector;
    D_L00_00161108_m.size = size;
    D_L00_00161108_m.unk8 = arg2;
    D_L00_0015F5D8 = 1;
    func_001F4A58(4);
    func_L00_002039A0();
    D_L00_0015F5C4 = 1;
    D_L00_00161108_m.buf0 = D_L00_00173E04;
    D_L00_00161108_m.buf1 = D_L00_0015F40C;
    big = D_0015ED80 != 0 ? 0xE : 0xD;
    small = D_0015ED80 != 0 ? 0xE : 0x11;
    src = D_L00_00161108_m.buf1;
    gs_addr = D_0015EF78_29A940[4];
    i = 0;
    while (i < big) {
        func_00122330(image, gs_addr >> 8, 2, 1, 0, 0, 0x80, 0x80);
        i++;
        gs_addr += 0x10000;
        func_00118A80(0);
        func_00122658(image, (void *)src);
        src += 0xC000;
        func_00120558(0, 0);
    }
    gs_addr = D_0015EF78_29A940[0];
    j = 0;
    while (j < small) {
        func_00122330(image, gs_addr >> 8, 1, 0, 0, 0, 0x40, 0x40);
        j++;
        gs_addr += 0x4000;
        func_00118A80(0);
        func_00122658(image, (void *)src);
        src += 0x4000;
        func_00120558(0, 0);
    }
    if (gs_addr > 0x400000) {
        func_001E93B0(D_L00_001E9480, gs_addr - 0x400000);
        func_001F97B0();
    }
}
