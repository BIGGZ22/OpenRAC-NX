#include "include_asm.h"

/* Near miss: func_002859C8 (boot 0x2859C8, 3436 bytes, in every level too).

   This C compiles to 3436 bytes, the same size as retail, with the same frame
   (0x4D0, s0-s5/ra and f20-f22 saved). 843 of the 859 instructions align with
   retail once relocated operands are masked; `tools/check.py` stops at +0xA50.
   It has to be compiled with -G8 (as src/game/00285840-g8.c): the $gp loads at
   +0x8D8 and +0xA90.. are in ordinary flow.

   What still differs (scheduling and allocation only):
   - Cases 9, 10 and 11 (the three identical func_002802E0 panel calls):
     retail leaves the ALU idle in the first cycles and computes `&box` (and its
     copy to $a0) after the stack-argument store, with the add.s two slots
     earlier; here sched2 issues `&box` in cycle 2. Tried with no change: the
     rounded alpha in a separate int or float variable, a local MenuBox pointer,
     both operand orders, and four inline-helper shapes (those shrink the body).
   - Case 11: retail compares S.f58 against 1 held in $s0 (set in the delay slot
     of the func_002802E0 call), which also puts `&S` ($s4) before the call; here
     the 1 is a fresh $v0 after the load. Retail behaves as if a pseudo holding 1
     already crossed that call; no natural source for it was found. In the same
     case, the S.f58 == 3 path orders `a2 = s2` before `a1 = s0` for sprintf and
     keeps text(0x113B) in $s2 (here $s0).

   Even once exact it cannot land yet: the outer switch (11 entries, 0x26C590)
   and the inner one in case 1 (23 entries, 0x26C5C0) are jump tables, and
   tools/build.py refuses a body that needs .rodata at its place (see
   func_00123D30 in nonmatching/README.md).

   What made the difference while matching, for the next attempt:
   - every case keeps its own block-scoped variables (one shared `id` or `text`
     becomes one pseudo and drags every case into callee-saved registers);
   - memset (func_00115484) must return void *, strncpy (func_00115AC0) char *
     and sprintf (func_00115DA8) int: the call setting $v0 steers the next value
     to $v1 (local-alloc's anti-false-dependency rule), which fixed the box
     builders, case 2's first byte test and several argument orders at once;
   - the SPR base is a register variable (`spr`), not a folded constant;
   - `show = 1` precedes the f5C test in case 1 (it lands in the blez delay slot);
   - inner cases 2 and 21 assign id1 after the text call, 17/18/20 before it,
     which is what decides retail's cross-jumped tails;
   - the plain small extern D_001A8C8C under -G8 gives retail's $gp load in a
     delay slot and lui/lw elsewhere; D_001F29A0 must be non-small (cc1 splits
     it, so its lui can sit in a delay slot on its own);
   - case 11's {0, 1, 2, 3} switch (0 and 2 just break) reproduces retail's
     decision tree; case 3's tail is func_00280190's body inlined.
*/

typedef struct {
    short x;
    unsigned short f02;
    short f04;
    short f06;
    short f08;
    short f0A;
    short f0C;
    short h;
    short f10;
    unsigned short flags;
    short f14;
    short f16;
} MenuBox;

typedef struct {
    char pad0[0x40];
    int mode;
    int f44;
    char pad48[0x10];
    int f58;
    int f5C;
    int f60;
    char pad64[0x18];
    short f7C;
    short f7E;
    int f80;
} MenuState;

extern MenuState D_001BAC80;
extern MenuBox D_001A8A88;
extern MenuBox D_001A8AB8;
extern char D_001A8AA0[];
extern char D_001A8AA8[];
extern char D_001A8AD0[];
extern char D_00263B50[];
extern char *D_001A8A4C __attribute__((sda));
extern char *D_001A8A50 __attribute__((sda));
extern char *D_001A8A54 __attribute__((sda));
extern char *D_001A8A58 __attribute__((sda));
extern int D_001A8A70 __attribute__((sda));
extern int D_001A8A74 __attribute__((sda));
extern int D_001A8C88;
extern int D_001A8C8C;
extern int D_001F29A0[];
extern int D_001A7420;
extern unsigned short D_001A7344;
extern int D_001B1598;
extern float D_001B23A4;

extern void func_0027DDC0(int, int, int, int);
extern void func_0027C540(int);
extern void func_002FC560(unsigned int, unsigned long);
extern char *func_00288FB8(int);
extern char *func_00115AC0(unsigned char *, char *, int);
extern void func_00280190(MenuBox *, unsigned long, unsigned char *, int);
extern int func_001157AC(unsigned char *);
extern void func_0029B888(MenuBox *, int, void *, int, int, int, int, char *, char *, char *);
extern int func_0028E588(int, int);
extern void func_0028E868(int, int, int, int, int, int);
extern int func_0028E640(int);
extern void func_0028F410(float, float, float, float, float, int, int, int);
extern void *func_00115484(void *, int, int);
extern int func_00288D80(void);
extern int func_00287E90(void);
extern int func_00115DA8(char *, const char *, ...);
extern void func_0027EB88(int, int, int, int, int, int);
extern int func_00283D38(int, int, float);
extern void func_0027FAA8(int, int, unsigned long, char *, int);
extern int func_0027C878(int);
extern void func_0027FC70(MenuBox *, unsigned long, char *, int, int, void *);
extern void func_002802E0(MenuBox *, int, int, int, int, int, int, int, int);
extern void func_0027C660(void);

#define SPR ((unsigned char *)0x70000000)

void func_002859C8(void)
{
    char buf[0x300];
    MenuBox box;
    MenuBox tmp;
    MenuBox tmp2;
    char buf2[0x100];

    if (D_001BAC80.f7E == 0)
        func_0027DDC0(0, 0, 0, 0x30);
    if (D_001BAC80.f7C != 0)
        return;
    func_0027C540(0);
    func_002FC560(0x42, 0x8000000044UL);
    func_002FC560(0x47, 0x5360B);
    switch (D_001BAC80.mode) {
    case 2: {
        unsigned char *spr;
        unsigned char *p;
        int h, len, i, x, y;

        box = D_001A8A88;
        spr = SPR;
        func_00115AC0(spr, func_00288FB8(0x9B4), 0x400);
        p = spr;
        while (*p >= 2)
            p++;
        while (*p == 1) {
            *p = 0;
            p++;
        }
        box.flags |= 4;
        func_00280190(&box, 0x80000000UL, spr, -1);
        box.flags ^= 4;
        h = box.h;
        len = func_001157AC(spr);
        for (i = 4; i >= 0; i--)
            spr[len + i] = 1;
        spr[len + 5] = 0;
        func_0029B888(&box, 0x80F0F0F0, spr, 1, 1, -1, 1, func_00288FB8(0x1137), 0, 0);
        x = box.x + h;
        func_002FC560(0x47, 0x3004B);
        y = x + 8;
        func_0028E868(func_0028E588(0x755D, 0), 0xE0, y, 0x40, 0x40, 0x80);
        func_0028F410(4096.0f, (x + 40) * 16, 272.0f, 272.0f,
                      (D_001B1598 % 55) * -0.11423973f, 0x40, 0x40,
                      func_0028E640(func_0028E588(0x755D, 1)));
        break;
    }
    case 4:
        if (D_001BAC80.f5C > 0) {
            unsigned char *p;
            int id;

            func_00115AC0(SPR, func_00288FB8(0x2DE5), 0x400);
            p = SPR;
            while (*p >= 2)
                p++;
            while (*p == 1) {
                *p = 0;
                p++;
            }
            id = D_001BAC80.f44 ? 0 : 0x1137;
            func_00115484(&tmp, 0, sizeof(tmp));
            tmp.f02 = D_001A7344;
            tmp.f04 = 0x60;
            tmp.f06 = 0x1A0;
            tmp.f08 = 0x100;
            tmp.f0A = 0x68;
            tmp.f10 = 0x10;
            tmp.flags = 1;
            box = tmp;
            func_0029B888(&box, 0x80F0F0F0, p, 1, 1, -1, 0, id ? func_00288FB8(id) : 0, 0, 0);
        }
        break;
    case 5:
        if (D_001BAC80.f5C > 0) {
            char *text;
            int id;

            text = func_00288FB8(D_001BAC80.f80 ? 0x2E04 : 0x2E03);
            id = D_001BAC80.f44 ? 0 : 0x1137;
            func_00115484(&tmp2, 0, sizeof(tmp2));
            tmp2.f02 = D_001A7344;
            tmp2.f04 = 0x60;
            tmp2.f06 = 0x1A0;
            tmp2.f08 = 0x100;
            tmp2.f0A = 0x68;
            tmp2.f10 = 0x10;
            tmp2.flags = 1;
            box = tmp2;
            func_0029B888(&box, 0x80F0F0F0, text, 1, 1, -1, 0, id ? func_00288FB8(id) : 0, 0, 0);
        }
        break;
    case 1: {
        int show, id1, id2;
        char *text;

        show = 1;
        if (D_001BAC80.f5C > 0) {
            text = D_001A8AA0;
            id1 = 0;
            id2 = 0;
            if (func_00288D80()) {
                id1 = 0x1137;
                text = func_00288FB8(0x2E04);
            } else {
                switch (D_001A7420) {
                case 6:
                    id1 = 0x113C;
                    id2 = 0x1138;
                    text = func_00288FB8(0x2DF4);
                    break;
                case 12:
                    if (D_001F29A0[0]) {
                        id1 = 0x2DE8;
                        text = func_00288FB8(0x2DE6);
                        break;
                    }
                case 13:
                    id1 = 0x113C;
                    id2 = 0x1138;
                    text = func_00288FB8(0x2DF0);
                    break;
                case 10:
                case 11:
                    text = func_00288FB8(0x2E00);
                    break;
                case 7:
                case 8:
                    text = func_00288FB8(0x2DFE);
                    break;
                case 14:
                case 15:
                    text = func_00288FB8(0x2DFF);
                    break;
                case 17:
                    id1 = 0x1137;
                    text = func_00288FB8(0x2E01);
                    break;
                case 18:
                    id1 = 0x1137;
                    text = func_00288FB8(0x2E03);
                    break;
                case 2:
                    text = func_00288FB8(0x2DE5);
                    id1 = D_001BAC80.f44 ? 0 : 0x1137;
                    break;
                case 19:
                    if (D_001F29A0[0]) {
                        id1 = 0x2DE8;
                        text = func_00288FB8(0x2DE6);
                    } else if (D_001A8C88) {
                        id1 = 0x113B;
                        id2 = 0x1138;
                        func_00115DA8(buf, D_001A8AA8, func_00288FB8(0x2DE9), 1, 1,
                                      func_00288FB8(0x2DEB));
                        text = buf;
                    } else {
                        id1 = 0x113B;
                        func_00115DA8(buf, D_001A8AA8, func_00288FB8(0x2DE9), 1, 1,
                                      func_00288FB8(0x2DEB));
                        text = buf;
                        if (D_001A8C8C)
                            id1 = 0x113C;
                    }
                    break;
                case 21:
                    text = func_00288FB8((!D_001A8C88 && func_00287E90()) ? 0x2E04 : 0x2E03);
                    id1 = 0x1137;
                    break;
                case 20:
                    id1 = 0x1137;
                    text = func_00288FB8(0x2E02);
                    break;
                case 24:
                    id1 = 0x113B;
                    id2 = 0x1138;
                    func_00115DA8(buf, D_001A8AA8, func_00288FB8(0x2DF2), 1, 1,
                                  func_00288FB8(0x2DEB));
                    text = buf;
                    break;
                case 23:
                    id1 = 0x113B;
                    id2 = 0x1138;
                    text = func_00288FB8(0x2DF6);
                    break;
                case 3:
                case 4:
                    if (D_001A8C88 && !D_001F29A0[0]) {
                        id1 = 0x113B;
                        id2 = 0x1138;
                        text = func_00288FB8(0x2DEC);
                    } else if (D_001A8C8C) {
                        id1 = 0x113B;
                        text = func_00288FB8(0x2DEC);
                    } else if (D_001F29A0[0]) {
                        id1 = 0x2DE8;
                        text = func_00288FB8(0x2DE6);
                    } else {
                        id1 = 0x2DE8;
                        text = func_00288FB8(0x2DEE);
                    }
                    break;
                case 5:
                    id1 = 0x2DE8;
                    text = func_00288FB8(0x2DE6);
                    break;
                default:
                    show = 0;
                    break;
                }
            }
            if (show) {
                func_00115484(&tmp, 0, sizeof(tmp));
                tmp.f02 = D_001A7344;
                tmp.f04 = 0x60;
                tmp.f06 = 0x1A0;
                tmp.f08 = 0x100;
                tmp.f0A = 0x68;
                tmp.f10 = 0x10;
                tmp.flags = 1;
                box = tmp;
                func_0029B888(&box, 0x80F0F0F0, text, 1, 1, -1, 0,
                              id1 ? func_00288FB8(id1) : 0, id2 ? func_00288FB8(id2) : 0, 0);
            }
        }
        break;
    }
    case 3: {
        float t;
        int c1, c2, id;

        t = 1.0f - D_001BAC80.f44 * 0.033333335f;
        box = D_001A8AB8;
        func_0027EB88(0x64, 0x12C, 0x60, 0x1A0, (int)(t * 80.0f), 0x40404);
        c1 = func_00283D38(D_001A8A70, D_001A8A74, t);
        c2 = func_00283D38(0x20FFFF, 0x8020FFFF, t);
        switch (D_001BAC80.f58) {
        case 0:
        case 1:
            func_0027FAA8(0xCA, 0x118, c2, func_00288FB8(0x113B), -1);
            func_0027FAA8(0x135, 0x118, c2, func_00288FB8(0x1138), -1);
            id = 0x2F28;
            break;
        case 2:
            func_0027FAA8(0xCA, 0x118, c2, func_00288FB8(0x113B), -1);
            func_0027FAA8(0x135, 0x118, c2, func_00288FB8(0x1138), -1);
            id = 0x2F29;
            break;
        case 3:
            func_0027FAA8(0x100, 0x118, c2, func_00288FB8(0x1137), -1);
            id = 0x2F2A;
            break;
        default:
            id = 0;
            break;
        }
        if (id)
            func_0027FC70(&box, c1, func_00288FB8(id), -1, func_0027C878(1), D_00263B50);
        break;
    }
    case 9: {
        char *a, *b, *c;

        func_002802E0(&box, 0xF0, 0x1E0, 0x60, 0x1A0, 0x100,
                      (int)(D_001B23A4 * 100.0f + 0.5f), 0x10, 3);
        a = D_001A8A4C;
        if (!a)
            a = func_00288FB8(0x113B);
        b = D_001A8A50;
        if (!b)
            b = func_00288FB8(0x1138);
        c = D_001A8A54;
        if (!c)
            c = func_00288FB8(0x2DCD);
        func_0029B888(&box, 0x80F0F0F0, c, 1, 1, -1, 0, a, b, D_001A8A58);
        break;
    }
    case 10: {
        char *a, *c;

        func_002802E0(&box, 0xF0, 0x1E0, 0x60, 0x1A0, 0x100,
                      (int)(D_001B23A4 * 100.0f + 0.5f), 0x10, 3);
        a = D_001A8A4C;
        if (!a)
            a = func_00288FB8(0x113B);
        c = D_001A8A54;
        if (!c)
            c = func_00288FB8(0x2DCD);
        func_0029B888(&box, 0x80F0F0F0, c, 1, 1, -1, 0, a, 0, D_001A8A58);
        break;
    }
    case 11:
        func_002802E0(&box, 0xF0, 0x1E0, 0x60, 0x1A0, 0x100,
                      (int)(D_001B23A4 * 100.0f + 0.5f), 0x10, 3);
        switch (D_001BAC80.f58) {
        case 0:
            break;
        case 1: {
            char *a, *b;

            a = func_00288FB8(0x1137);
            b = func_00288FB8(0x2805);
            func_00115DA8(buf2, func_00288FB8(0x31E6), 15);
            func_0029B888(&box, 0x80F0F0F0, buf2, 1, 1, -1, 0, a, b, 0);
            break;
        }
        case 2:
            break;
        case 3:
            D_001BAC80.f60 = D_001BAC80.f5C / 60 + 1;
            func_00115DA8(buf2, D_001A8AD0, func_00288FB8(0x31ED), func_00288FB8(0x31EE),
                          D_001BAC80.f60, func_00288FB8(0x31EF));
            func_0029B888(&box, 0x80F0F0F0, buf2, 1, 1, -1, 0, func_00288FB8(0x113B),
                          func_00288FB8(0x1138), 0);
            break;
        }
        break;
    }
    func_0027C660();
}
