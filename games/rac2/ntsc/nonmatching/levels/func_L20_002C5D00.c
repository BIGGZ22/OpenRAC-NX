/* Near miss from src/levels/20_yeedil/L20_002B7A48-g8.c: the body is 1248 bytes, the
   retail's size, but the prologue already differs (this build saves one callee-
   saved register too many, so the frame is 0x50 instead of 0x40) and everything
   after it is shifted.  Kept as a starting point; not linked, not counted.

   This function belongs in the -G8 unit (src/levels/20_yeedil/L20_002B7A48-g8.c):
   it reaches D_L20_001AF000 through $gp ("lw $v0,0xF10($gp)").

   THE SHAPE OF THE RETAIL BODY (the map a next attempt should follow):

     - the prologue keeps three callee-saved registers ($s0, $s1, $s2) and one
       float ($f20, the 0.0 constant): $s0 is the table entry, $s1 is the 64-bit
       %hi of D_00189E20 ("daddu $s1,$v0,$zero") and $s2 is the argument, widened
       to 64 bits ("daddu $s2,$a0,$zero").  The hero block itself lives in $a2 in
       the first block only and is re-materialised from $s1 later
       ("addiu $reg,$s1,%lo(D_189E20)"), so the source must NOT keep a pointer
       to it across a call - that is what costs the extra saved register here.

     - the early exits, in order: base->0x229C == 0x14, base->0x229C == 7,
       base->0x2294 == 0x31, base->0x1DC (a halfword) != 0, D_001A8F00 != 0,
       then base->0x249C = 0, then the hero object at base->0x2290, its byte at
       +0xA8 (0xFF ends it), the table D_001C4680 indexed by that byte times 64
       ("sll $v0,$v0,6" + "addu $s0,$v0,$v1"), and the table entry's +0x38 must
       be the hero object.

     - the state byte at base+0x22B4 picks the way the entry's +0x2C float is
       used: the "== 4" path checks D_001AFF00 and base->0x2294 != 0x6F, the
       other path checks (entry->0x24 & 9) != 0.  The float is then compared
       with 0.0 ("c.eq.s $f0,$f20" + "bc1t"), which sets D_001A7A33 = 0 and
       D_001A7A32 = 1 when it is not zero.

     - the call to func_L00_0037E668 is followed by a second test of the same
       float, and by the state byte test at D_00189E20+0x22B4 == 0, which calls
       func_L00_0037E5F0 with the entry's +0x2C, compares the float with 0.0
       again (that one returns 0) and stores the float back into the entry.

     - then entry->0x20 is stored into D_00189E20+0x249C, and if
       D_00189E20+0x229C == 0xF the entry is stored into +0x6C4, 1 into +0x68E
       and the function returns 0.

     - the state byte == 4 test with D_L20_001AF000 == 0 and
       D_00189E20+0x2294 != 0x6F takes the halfword at hero->0x68+0x3BC,
       subtracts 0xB4 from it and, when it went negative, stores it and then
       stores 0 into the same place.

     - the argument is tested against 0 (that returns 1), func_L00_002A6708 is
       called and the argument is set to 0.  Then entry->0x30 & 1 picks between
       the 128-bit copy of entry->0x10 (into the stack slot) and the call to
       func_00282CF0 with D_00189EA0, or func_L00_002D36E0 with -1.0f and 0.0f.
       The float entry->0x1C is compared with 5627.924804688f (0x45AFDF66).

     - the last block is a switch on the state byte with the cases 1, 0 and 4
       and a default; every path ends in the two calls to func_00282CC0 with
       D_00189F10 and D_00189F10+0x20 and returns 1.

   The float constants of the retail, for the record: 0x45AFDF66 = 5627.924804688f,
   0xBF800000 = -1.0f, 0x3D6147AF = 0.055000003f, 0x3CE81B50 = 0.028333336f,
   0x3DAAAAAB = 0.083333336f, 0x3D4CCCCE = 0.050000004f, 0x40000000 = 2.0f.

   The calls, by their level-20 address: 0x396798 func_L00_0037E668,
   0x396720 func_L00_0037E5F0, 0x2B9610 func_L00_002A6708, 0x302290 func_00282CF0,
   0x2E7608 func_L00_002D36E0, 0x302260 func_00282CC0, 0x2D7730 func_L20_002D7730,
   0x331BB8 func_002A9640, 0x3033C8 func_00283CF0, 0x2B7B10 func_L00_002A4EF8,
   0x2CCCD8 func_L00_002B9B88, 0x2C5C90 func_L00_002B2C58, 0x2E7B90
   func_L00_002D3C68.

   What still differs in this attempt: the prologue (one saved register too
   many, the frame 0x50 instead of 0x40) and, behind it, the whole body.  The
   widening of the argument to 64 bits is "move $s2,$a0" here where the retail
   has "daddu $s2,$a0,$zero"; a cast of a pointer to a 64-bit type gives the
   move, and a cast of an unsigned 32-bit value gives the "dsll32/dsrl32" pair,
   so the spelling that gives the DADDU is still open.

   Measured with tools/check.py --diff: the retail's size is 1248 bytes, this
   build is 1148 bytes and the prologue already differs. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef u64 TI128 __attribute__((mode(TI)));

extern s32 D_L20_001AF000 __attribute__((sda));
extern s32 D_001A8F00;
extern s32 D_001AFF00;
extern u8 D_001A7A32;
extern u8 D_001A7A33;
extern char D_001C4680[];
extern char D_00189E20[];
extern char D_00189EA0[];
extern char D_00189F10[];

extern void func_L00_0037E668(void);
extern void func_L00_0037E5F0(f32);
extern void func_L00_002A6708(void);
extern void func_00282CF0(TI128 *, void *, void *);
extern void func_L00_002D36E0(void *, void *, f32, f32);
extern void func_00282CC0(void *, void *, void *);
extern void func_L20_002D7730(s32, s32);
extern s32 func_002A9640(s32, s32, s32);
extern void func_00283CF0(f32);
extern void func_L00_002A4EF8(s64);
extern void func_L00_002B9B88(s32, void *);
extern void func_L00_002B2C58(void *, s64, void *);
extern void func_L00_002D3C68(void *, void *);

s32 func_L20_002C5D00(char *arg0)
{
    char *base;
    char *ent;
    s64 p;
    s32 v;
    s32 w;
    TI128 tmp;

    base = D_00189E20;
    p = (s64)arg0;

    if (*(s32 *)(base + 0x229C) == 0x14)
        return 0;
    if (*(s32 *)(base + 0x229C) == 7)
        return 0;
    if (*(s32 *)(base + 0x2294) == 0x31)
        return 0;
    if (*(s16 *)(base + 0x1DC) != 0)
        return 0;
    if (D_001A8F00 != 0)
        return 0;

    *(s32 *)(base + 0x249C) = 0;
    v = *(u8 *)(*(char **)(base + 0x2290) + 0xA8);
    if (v == 0xFF)
        return 0;
    ent = D_001C4680 + v * 64;
    if (*(char **)(ent + 0x38) != *(char **)(base + 0x2290))
        return 0;

    if (*(u8 *)(base + 0x22B4) == 4) {
        if (D_001AFF00 != 0)
            return 0;
        if (*(s32 *)(base + 0x2294) == 0x6F)
            return 0;
    } else {
        if ((*(s32 *)(ent + 0x24) & 9) == 0)
            return 0;
    }

    if (*(f32 *)(ent + 0x2C) != 0.0f) {
        D_001A7A33 = 0;
        D_001A7A32 = 1;
    }
    func_L00_0037E668();
    if (*(f32 *)(ent + 0x2C) != 0.0f) {
        if (*(u8 *)(D_00189E20 + 0x22B4) == 0) {
            func_L00_0037E5F0(*(f32 *)(ent + 0x2C));
            if (*(f32 *)(ent + 0x2C) == 0.0f)
                return 0;
            *(f32 *)(ent + 0x2C) = *(f32 *)(ent + 0x2C);
        }
    }
    v = *(s32 *)(ent + 0x20);
    {
        s32 st = *(s32 *)(D_00189E20 + 0x229C);

        *(s32 *)(D_00189E20 + 0x249C) = v;
        if (st == 0xF) {
            *(s32 *)(D_00189E20 + 0x6C4) = (s32)ent;
            *(u16 *)(D_00189E20 + 0x68E) = 1;
            return 0;
        }
    }

    if (*(u8 *)(D_00189E20 + 0x22B4) == 4 && D_L20_001AF000 == 0 &&
        *(s32 *)(D_00189E20 + 0x2294) != 0x6F) {
        char *q = *(char **)(*(s32 *)(D_00189E20 + 0x2290) + 0x68);
        s16 h = *(u16 *)(q + 0x3BC);

        h -= 0xB4;
        if (h < 0) {
            *(u16 *)(q + 0x3BC) = h;
            *(u16 *)(q + 0x3BC) = 0;
        }
    }

    if (p == 0)
        return 1;
    func_L00_002A6708();
    p = 0;

    if ((*(s32 *)(ent + 0x30) & 1) != 0) {
        TI128 *src = (TI128 *)(ent + 0x10);

        tmp = *src;
        if (*(f32 *)(ent + 0x1C) == 5627.924804688f)
            goto skip1;
        p = 1;
    } else {
        v = *(s32 *)(ent + 0x20);
        if (v != 0)
            func_00282CF0(&tmp, D_00189EA0, (void *)(v + 0x10));
        else
            func_L00_002D36E0(&tmp, 0, -1.0f, 0.0f);
    }
skip1:
    switch (*(u8 *)(D_00189E20 + 0x22B4)) {
    case 1:
        func_L20_002D7730(0x42, 1);
        func_L00_002B2C58(&tmp, p, &tmp);
        goto out;
    case 0:
        w = *(s32 *)(D_00189E20 + 0x249C);
        if (w == 0)
            goto out2;
        if (*(s16 *)(w + 0xAA) == 0x4EB || *(s16 *)(w + 0xAA) == 0x558) {
            func_L20_002D7730(0x7D, 1);
            return 0;
        }
        if (*(s32 *)(D_00189E20 + 0x2294) != 1 && *(s32 *)(D_00189E20 + 0x2294) != 0x1D) {
            if (*(s32 *)(D_00189E20 + 0x1500) == 0)
                goto out3;
        }
        w = *(s32 *)(ent + 0x24);
        if ((w & 8) != 0)
            goto out3;
        if (func_002A9640(*(s32 *)(D_00189E20 + 0x2290), 1, 0) != 0) {
            func_00283CF0(*(f32 *)((char *)w + 0x2C));
            func_L00_002A4EF8(w);
            w = *(s32 *)(D_00189E20 + 0x24CC);
        }
        if (w == 0) {
            func_L20_002D7730(0x16, 1);
            w = 0x4D;
        }
        if (*(s16 *)(D_00189E20 + 0x21C) == 0) {
            *(u16 *)(D_00189E20 + 0x1DC) = w;
            *(u16 *)(D_00189E20 + 0x21C) = 0x12;
        }
        func_L00_002B9B88(0x14, 0);
        return 0;
    case 4:
        func_L20_002D7730(0x90, 1);
        func_L00_002B2C58(&tmp, p, &tmp);
        break;
    default:
        break;
    }
out3:
    if (*(s32 *)(D_00189E20 + 0x229C) == 0x12) {
        func_L20_002D7730(0x11, 1);
    } else if (*(s32 *)(D_00189E20 + 0x229C) != 0x11) {
        if ((*(s32 *)(ent + 0x24) & 8) == 0)
            func_L20_002D7730(0x16, 1);
        else
            func_L20_002D7730(0x8C, 1);
    } else {
        func_L20_002D7730(0x8C, 1);
    }
    if (*(s32 *)(D_00189E20 + 0x2294) != 0x8C) {
        tmp = 0;
        goto out2;
    }
    func_L00_002B2C58(&tmp, p, &tmp);
    if (p == 0)
        goto out2;
    if (*(u8 *)(ent + 0x28) == 4) {
        func_L00_002D3C68(&tmp, &tmp);
        goto out2;
    }
out:
    func_00282CC0(D_00189F10, D_00189F10, &tmp);
    func_00282CC0(D_00189F10 + 0x20, D_00189F10 + 0x20, &tmp);
    return 1;
out2:
    func_00282CC0(D_00189F10, D_00189F10, &tmp);
    func_00282CC0(D_00189F10 + 0x20, D_00189F10 + 0x20, &tmp);
    return 1;
}
