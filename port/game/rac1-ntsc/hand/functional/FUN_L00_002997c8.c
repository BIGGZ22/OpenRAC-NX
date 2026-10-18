/* Carried from the PAL decompilation's nonmatching/functional/func_L00_0029AB38.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * Checked against the US assembly (2026-10-18): the music call is FUN_00215c40 (level 0's jal 0x265418 at 0x29993C, a2 = 0x400), as carried; an earlier "correction" to 00215B68 was wrong. */
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
/* functional: medium. MovieExitToGameplay: stops the movie, re-uploads the saved frame buffer images in 13/14 + 17/14 strips (16-bit shifted source addresses) to the buffers at D_L00_00161058, restores music/trigger state and sets bit 0x10 in a byte of the voice pool header. Checked against the US assembly: the music call is func_00215C40 (jal 0x265418 in level 0, which functions.tsv places at FUN_00215c40). */
extern void f122598_9AB38(int) __asm__("func_00122298");
extern void f120858_9AB38(int, int) __asm__("func_00120558");
extern void f123168_9AB38(void *) __asm__("func_00122E68");
extern void f12F308_9AB38(void) __asm__("func_0012F1C8");
extern void f122818_9AB38(void *, int, int, int, int, int, int, int) __asm__("func_00122518");
extern void f118D80_9AB38(int) __asm__("func_00118A80");
extern void f122AD8_9AB38(void *, int) __asm__("func_001227D8");
extern void f1F4E08_9AB38(int) __asm__("func_001F4A58");
extern void f215B68_9AB38(int, int, int) __asm__("func_00215C40");
extern void f217AE8_9AB38(int, int, int) __asm__("func_00216C48");
extern int D_L00_0015F6BC_9AB38 __asm__("D_L00_0015F5D8") MACRO_ADDR;
extern int D_0015EE80_9AB38 __asm__("D_0015ED80") MACRO_ADDR;
extern int D_L00_00161118_9AB38 __asm__("D_L00_00161058") MACRO_ADDR;
extern int D_0015EF88_9AB38 __asm__("D_0015EE88") MACRO_ADDR;
extern int D_0015EF78_9AB38 __asm__("D_0015EE78") MACRO_ADDR;
extern int D_0015EFD8_9AB38 __asm__("D_0015EED8") MACRO_ADDR;
extern int D_L00_0015F6A8_9AB38 __asm__("D_L00_0015F5C4") MACRO_ADDR;
extern int D_L00_00179200_9AB38[] __asm__("D_L00_00179100") __attribute__((section(".data")));
extern char D_0014171B_9AB38[] __asm__("D_0014161B") __attribute__((section(".data")));
extern char D_0013E633_9AB38[] __asm__("D_0013E533") __attribute__((section(".data")));


void FUN_L00_002997c8(void) {
    unsigned char buffer[0x70] __attribute__((aligned(16)));
    int source, destination, count, second_count, pal;
    int cb, cb_arg;

    D_L00_0015F6BC_9AB38 = 1;
    f122598_9AB38(0);
    f120858_9AB38(0, 0);
    f123168_9AB38((void *)f12F308_9AB38);
    pal = D_0015EE80_9AB38;
    count = 0xD;
    second_count = 0x11;
    destination = D_L00_00161118_9AB38;
    source = D_0015EF88_9AB38;
    if (pal != 0) count = 0xE;
    if (pal != 0) second_count = 0xE;
    for (; count != 0;) {
        count--;
        f122818_9AB38(buffer, (source << 8) >> 16, 2, 1, 0, 0, 0x80, 0x80);
        source += 0x10000;
        f118D80_9AB38(0);
        f122AD8_9AB38(buffer, destination);
        destination += 0xC000;
        f120858_9AB38(0, 0);
    }
    source = D_0015EF78_9AB38;
    if (second_count != 0) {
        count = second_count;
        do {
            count--;
            f122818_9AB38(buffer, (source << 8) >> 16, 1, 0, 0, 0, 0x40, 0x40);
            source += 0x4000;
            f118D80_9AB38(0);
            f122AD8_9AB38(buffer, destination);
            destination += 0x4000;
            f120858_9AB38(0, 0);
        } while (count != 0);
    }
    if (D_0015EFD8_9AB38 == 2)
        D_0015EFD8_9AB38 = 0;
    f1F4E08_9AB38(4);
    f215B68_9AB38(*(short *)(D_0014171B_9AB38 + 0x100B5 + 0x38), 1, 0x400);
    D_L00_0015F6A8_9AB38 = 0;
    cb = D_L00_00179200_9AB38[2];
    if (cb != 0) {
        cb_arg = D_L00_00179200_9AB38[3];
        D_L00_00179200_9AB38[3] = 0;
        D_L00_00179200_9AB38[2] = 0;
        f217AE8_9AB38(cb, cb_arg, 1);
    }
    D_0013E633_9AB38[0x1D + 0x6B] |= 0x10;
}
