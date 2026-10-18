#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * freeze.cpp in the original source; text 0x1FBC80-0x1FDF10.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;

extern void func_0012E528(int);
extern void func_00216EF0(int);
extern int func_001FE540(int);
extern void func_001FF560(int);
extern int func_001F98C0(int);
extern int D_0015F6E8 MACRO_ADDR;
extern int D_0013D440;
extern char D_00193400[];

/* Enters the freeze screen for reason arg0: pauses the music unless the
   game mode (D_0015F6E8) is already 3, saves arg1/arg2 and the old mode in
   the freeze record D_00193400, sets mode 4, then per reason fetches the
   message strings (func_001FE540) or starts the timers (func_001F98C0);
   an unknown reason gets a plain 0x78 countdown. Each case reads the
   record through its own `char *` local (retail keeps only the %hi and
   rebuilds the %lo per case); the case bodies are in retail's layout
   order, and the last store of a block is written last in the source
   because the scheduler issues it first. */
void Freeze_Enter(int arg0, int arg1, int arg2) {
    char *s = D_00193400;

    *(int *)(s + 0x30) = arg2;
    if (D_0015F6E8 != 3) {
        func_0012E528(0x1D);
        music_Pause(0);
    }
    *(int *)(s + 0x18) = arg1;
    *(int *)(s + 0x14) = D_0015F6E8;
    D_0015F6E8 = 4;
    *(int *)s = arg0;
    *(int *)(s + 0x2C) = 0;
    switch (arg0) {
    case 0: {
        char *p = D_00193400;
        *(int *)(p + 8) = msg_string(0x4F6E);
        *(int *)(p + 0xC) = msg_string(0x524D);
        *(int *)(p + 0x10) = msg_string(0x524E);
        *(int *)(p + 4) = 0;
        *(int *)(p + 0x1C) = 0;
        *(int *)(p + 0x20) = 0;
        *(int *)(p + 0x24) = 0;
        *(int *)(p + 0x28) = 0;
        break;
    }
    case 2: {
        int v = msg_string(0x524F);
        char *p = D_00193400;
        *(int *)(p + 8) = v;
        *(int *)(p + 0xC) = 0;
        *(int *)(p + 4) = 0;
        break;
    }
    case 1:
    case 4: {
        char *p = D_00193400;
        *(int *)(p + 8) = msg_string(0x522E);
        *(int *)(p + 0xC) = msg_string(0x4EE0);
        *(int *)(p + 0x10) = msg_string(0x524F);
        *(int *)(p + 4) = 0;
        break;
    }
    case 5: {
        char *p = D_00193400;
        PushHelpHistory(0x4E2B);
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = 0;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        break;
    }
    case 7:
    case 8: {
        char *p = D_00193400;
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = -1;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        break;
    }
    case 3: {
        char *p = D_00193400;
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = -1;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        D_0013D440 = 3;
        *(int *)(p + 0x2C) = func_001F98C0(0xA);
        break;
    }
    case 6: {
        int v = func_001F98C0(0x1E);
        char *p = D_00193400;
        *(int *)(p + 4) = v;
        *(int *)(p + 0x1C) = 0;
        break;
    }
    default:
        *(int *)(s + 4) = 0x78;
        *(int *)(s + 0xC) = 0;
        *(int *)(s + 0x10) = 0;
        *(int *)(s + 8) = 0;
        break;
    }
}
__asm__(".section .text\n\tnop\n");

INCLUDE_ASM("asm/nonmatchings/text", func_001FBE80);

INCLUDE_ASM("asm/nonmatchings/text", func_001FD3E8);
