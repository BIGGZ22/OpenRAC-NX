/* Best attempt for func_L20_00449850 (404 bytes, 1 place) - NOT MARKED.
   G8: this body reaches $gp-relative data (D_L20_001B23E0, D_L20_001B23E4,
   D_L20_001B2AC8) and needs the -G8 profile, so it belongs in the -G8
   sibling of its unit (src/levels/20_yeedil/L20_00449178-g8.c), not in
   L20_00449178.c.

   last check.py line: "20_yeedil: instruction at +0x84 differs"
   (the size is the retail 404 bytes now; 14 instructions still differ.)

   The shape is right - the two-mode dispatch, the mode 0 reset block, the
   mode 2 call, the $gp and absolute reads of the same word (0x001B2AC8),
   the float path and the two tail calls are all the retail instructions.
   What differs:

   - the constant 1: retail materialises it three times ($v0 for the
     "mode == 1" test, $a1 for the state+0x24 store, $v0 for the
     state+0xAC store); this cc1 CSEs the two stores' 1 into one value
     that is live across the f_00339E70 call, so it lands in $s2 and the
     frame grows by the $s2 save.
   - the same happens with a float: retail has no FPR save at all, this
     cc1 keeps one in $f20.
   - the branch order around the mode tests differs: retail tests
     "mode == 1" first, then "mode >= 2", then "mode == 0", and reuses
     the "mode < 2" value ($v0 = 0) for the "mode == 2" test, which is
     a comparison the source can never make true on that path.

   Tried, all landing on the same 396 bytes: the mode 0 block before and
   after the mode 2 block, the float result in a variable and used
   directly, the "1" written as a cast, the state+0xAC word read twice
   (as retail does) and through a variable, and the mode dispatch written
   as a switch.

   ------------------------------------------------------------------
   Second attempt (wave w20-11): the size is now EXACT (404 bytes) and 14
   instructions still differ.  What the second attempt measured, for the
   next worker:

   - the constant 1: `*(volatile u32 *)(state + 0xAC) = 1;` stops cc1 from
     sharing the constant with the earlier `*(u16 *)(state + 0x24) = 1;`.
     Without the volatile, cc1 keeps one 1 in $s2 across the f_00339E70
     call (the frame grows to 0x30).  With it, cc1 re-materialises the 1
     at the store, as retail does, and the frame is retail's 0x20 with
     $s0/$s1/$ra only.  The volatile store costs nothing in the bytes.

   - `nosda` on an EXTERNAL global changes the code: with it cc1 emits
     `lui $v0,%hi(S); lw $a0,%lo($v0)($v0)` and leaves the load after the
     prologue's saves; WITHOUT it (no attribute at all) cc1 emits retail's
     `lui $v1,%hi(S); lw $v1,%lo(S)($v1)` and hoists it to the top of the
     function, before the frame setup.  So an absolute global in a -G8
     file wants NO attribute; only the $gp ones want `sda`.  (Measured on
     D_L20_001A8F00: with nosda the function was 416 bytes, without it
     404.)

   - the mode dispatch: `if (mode >= 2) goto B8; if (mode == 0) goto C8;
     goto CHECK;` with the bodies behind labels reproduces retail's block
     order (dispatch, tail stub, B8, C8) and its `slti`+`beqz`.  `mode`
     must be a SIGNED int: a u8 gives `sltu` where retail has `slti`.

   - the call to f_00339E70 (func_L01_0031AB20) wants the interleaved
     prototype `(float, char *, float, char *, float, int)`: retail
     materialises the arguments as f12, a0, f13, a1, a2, a3, t0..t3, and
     the natural prototype gives a2 first.

   - 0x0018A3E0 is boot resident data (below 0x1A7C80), so its name is
     D_0018A3E0, not D_L20_0018A3E0.

   What still differs (14 instructions, all register choice or scheduling):

   - the three reads of D_0018A3E0 (the tail stub, B8 and the mode 2
     path): cc1 puts the address in $v1 and the value in $v0, retail puts
     the address in $v0 and the value in $v1.
   - the D_L20_001B2AC8 block: cc1 loads the $gp word into $a0 and the
     absolute word into $a0, retail uses $v1 for both, and retail stores
     0xAC before the `lui` while cc1 stores after it.
   - the two `andi` instructions are swapped: retail masks the pointer
     first, cc1 masks the global first.
   - the mode 0 block: retail loads D_001B23E0 ($f2) and D_001B23E4
     ($f0) before the `ori` and materialises the 1.0f ($f1) after it, and
     stores in the order 0x88, 0x20, 0x24, 0x70, 0x84 (the last one in
     the jal's delay slot); cc1 loads D_001B23E4 first, materialises the
     1.0f first and stores 0x70 first.  No permutation of the five
     statements (with or without temporary variables) reproduces retail's
     store order, so the source must be shaped differently.

   Flags tried on the -G8 unit, none of them closer: -fno-schedule-insns,
   -fno-schedule-insns2, -mno-split-addresses, -g, -O1, -O3.

   ------------------------------------------------------------------
   Third attempt (wave w21, worker 10): the attempt above is the best
   one so far - the size is the retail 404 bytes - and it is reproduced
   here with the four remaining differences measured again:

   - the three reads of D_0018A3E0: cc1 puts the ADDRESS in $v1 and the
     VALUE in $v0, retail puts the address in $v0 and the value in $v1.
     Reading D_0018A3E0 into a local first (`w = D_0018A3E0; if (w ==
     (int)arg0)`) changes nothing: the same two registers come out.
   - the D_L20_001B2AC8 pair: cc1 loads the $gp word into $a0 and the
     absolute word into $a0 as well; retail uses $v1 for both (the
     absolute one reuses the register of its own `lui`).  The `sw` of the
     0xAC word is before the `lui` on both sides, so only the register
     differs.
   - the two `andi` are still swapped: retail masks the pointer first
     (`andi $v0,$v0,0xF`), cc1 masks the global first, although the source
     compares the pointer side first.
   - the mode 0 block: retail loads D_L20_001B23E0 into $f2 and
     D_L20_001B23E4 into $f0 before the `ori` and materialises the 1.0f
     into $f1 after it, then stores 0x88, 0x20, 0x24, 0x70 and 0x84 (the
     last one in the jal's delay slot); cc1 loads D_L20_001B23E4 first,
     materialises the 1.0f first and stores 0x70 first.  Keeping the two
     globals in two locals (`a` then `b`) does give retail's LOAD order
     (D_L20_001B23E0 then D_L20_001B23E4) but the store order still
     follows the source, and the float registers come out as $f1/$f2
     where retail has $f2/$f0.

   The body must live in a -G8 unit (it reads three level globals through
   $gp), so the marker now sits in src/levels/20_yeedil/L20_00449850-g8.c
   (a file of its own, built with -G8 by its name) and the -G0 unit of
   L20_00449178.c only points at it.  The attempt below is the wave-3 one
   with the two locals added; it compiles to 404 bytes with the four
   differences above. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

/* an external global in a -G8 file: no attribute at all, that is what gives
   retail's absolute form */
extern int D_L20_001A8F00;

extern int D_0018A3E0 __attribute__((nosda));

extern float D_L20_001B23E0 __attribute__((sda));

extern float D_L20_001B23E4 __attribute__((sda));

extern int D_L20_001B2AC8 __attribute__((sda));

extern int D_001B2AC8;

extern void func_L20_00449A78(char *);

extern void func_L20_00449AE8(char *);

extern void func_L20_00449BB0(char *);

/* the interleaved prototype reproduces retail's argument setup order */
extern void func_L01_0031AB20(float, char *, float, char *, float, int);

extern float func_002A89A8(char *, int, float);

extern float func_00282C48(float);

extern int func_00282948(char *);

void func_L20_00449850(char *arg0) {
    int g = D_L20_001A8F00;
    char *state = *(char **)(arg0 + 0x68);
    int mode;
    int v;
    float f;
    float a, b;

    if (g == 2) {
        if (*(u8 *)(arg0 + 0x20) != 1) {
            func_L20_00449A78(arg0);
        }
    }
    mode = *(u8 *)(arg0 + 0x20);
    if (mode == 1) {
        return;
    }
    if (mode >= 2) {
        goto B8;
    }
    if (mode == 0) {
        goto C8;
    }
    goto CHECK;
B8:
    if (mode == 2) {
        goto G;
    }
    goto CHECK;
C8:
    a = D_L20_001B23E0;
    b = D_L20_001B23E4;
    *(float *)(state + 0x88) = b;
    *(float *)(state + 0x20) = 1.0f;
    *(u16 *)(state + 0x24) = 1;
    *(u32 *)(state + 0x70) |= 6;
    *(float *)(state + 0x84) = a;
    func_L20_00449A78(arg0);
    return;
G:
    func_L01_0031AB20(0.0f, arg0, 0.5f, state + 0x50, 0.6f, *(int *)(state + 0xA0));
    goto CHECK;
CHECK:
    v = D_L20_001B2AC8;
    if (D_0018A3E0 == (int)arg0) {
        *(volatile u32 *)(state + 0xAC) = 1;
        v = D_001B2AC8;
    }
    if ((((int)arg0 >> 8) & 0xF) == (v & 0xF)) {
        f = func_002A89A8(arg0 + 0x10, 0, 0.5f);
        f = *(float *)(arg0 + 0x18) - f;
        if (5.0f < func_00282C48(f)) {
            goto L499B8;
        }
    }
    if (*(u32 *)(state + 0xAC) == 0) {
        goto L499C8;
    }
    if (func_00282948(state + 0xA8) == 0) {
        goto L499C8;
    }
    goto L499B8;
L499B8:
    func_L20_00449AE8(arg0);
    goto END;
L499C8:
    func_L20_00449BB0(arg0);
END:
    return;
}
