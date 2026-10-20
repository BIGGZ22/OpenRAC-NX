/* Near miss (first attempt) from src/levels/20_yeedil/L20_0039C448-g8.c (the
   -G8 unit).

   This is a first attempt: it has the retail's instructions for the big "v == 1"
   block but not the retail's block order, and it is 616 bytes where the retail is
   648.  What is measured and known:

   - the outer dispatch of the retail is
       lbu  $v1,0x20($s0)
       beq  $v1,$v0(=1),.L003C0140      ; the v == 1 case, out of line
        lw  $s1,0x68($s0)
       slti $v0,$v1,0x2
       beql $v0,$zero,.L003C008C        ; the v >= 2 case, out of line, branch likely
        addiu $v0,$zero,0x2
       beql $v1,$zero,.L003C009C        ; the v == 0 case, out of line, branch likely
        lw  $v0,0xC($s1)
       b    .L003C02BC                   ; the return, the fall through
     so the source is "if (v == 1) { ... } else { if (v < 2) { ... } else { ... } }"
     with the two big cases out of line, and the comparison on the byte is
     SIGNED (`slti`, not `sltiu`), so the variable has to be signed.
     The branch-likely recipe applies: the small case falls through and the big
     one is the target of the branch-likely (see the wave's notes).
   - the constant 2 for the "v == 2" test is materialised in the delay slot of
     the branch-likely (`addiu $v0,$zero,0x2`) and used in the block the branch
     jumps to, so it is one variable in the source.
   - D_001A79F0 is read twice, once through $gp (at -0x7600) and once
     absolutely (`lui $a0,0x1A / lw $a0,0x79F0($a0)`), so it needs the
     double-name pattern (two C names for the one address).
   - D_L20_0019B4A8 and D_L20_0021EFB0 are used as 32-bit addresses
     (`lui`/`addiu`, not `daddu`), so the source casts them to 32 bits.
   - the indices are the sign extended halfword at 0xB2(arg0): `sll $a2,$a1,16 /
     sra $v1,$a2,16` in the v == 0 case and `sll $v0,$v1,16 / sra $v0,$v0,17`
     in the v == 1 case (the same value shifted once more).

   What is still different: the block order (the attempt keeps the v == 0 case
   in the fall through instead of putting it out of line behind the branch
   likely), the size (616 against 648 bytes) and the registers.

   The attempt belongs in the -G8 unit (it reaches D_001A79F0 through $gp).    With the attempt in src/ (the unit's default flags):
     20_yeedil: 636 bytes, expected 648
     func_L20_003C0048: 0 of 1 places match
*/

extern void func_003253A8(void *);

extern int func_0033E7F8(void *);

extern int func_00327008(u8);

extern float func_003033B8(int);

extern float func_00301ED0(void *);

extern void func_003C0350(void *);

extern void func_002C5A48(void *);

extern void func_0036E9D0(void *, void *);

extern void func_003C02D0(void *);

extern void func_004604E8(s32);

extern char *D_L20_001B2F9C __attribute__((nosda));

extern char D_L20_0019B4A8[] __attribute__((nosda));

extern char D_L20_0021EFB0[] __attribute__((nosda));

extern char D_L20_00189E20[] __attribute__((nosda));

extern int D_001A79F0;

extern int D_001A79F0_1 __attribute__((sda));

void func_L20_003C0048(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    s32 v = *(s32 *)(arg0 + 0x20);

    if (v != 1) {
        if (v < 2) {
            if (v != 0) {
                return;
            } else {
                s32 w = *(s32 *)(state + 0xC);

                if (w < 0 || *(s32 *)(state + 0x0) < 0) {
                    func_003253A8(arg0);
                    return;
                }
                if (*(s32 *)(state + 0x10) >= 0) {
                    u16 a1 = *(u16 *)(arg0 + 0xB2);
                    u8 c = *(u8 *)((s32)D_L20_0021EFB0 + (s16)a1);

                    if (c != 0) {
                        *(u8 *)(arg0 + 0x20) = 3;
                        return;
                    }
                    {
                        s32 g = D_001A79F0;
                        u8 b = *(u8 *)((s32)D_L20_0019B4A8 + ((a1 << 16) >> 17) + (g << 10));

                        if ((b & 7) != 0) {
                            *(u8 *)(arg0 + 0x20) = 3;
                            return;
                        }
                    }
                    *(u8 *)(arg0 + 0x20) = 1;
                    func_003C02D0(arg0);
                }
            }
        } else {
            if (v == 2) {
                func_00301ED0(state + 0x4);
                func_003C0350(arg0);
                if (*(s32 *)(state + 0x4) != 0) {
                    return;
                }
                {
                    char *s2 = (char *)D_L20_00189E20;

                    if (*(s32 *)(s2 + 0x2294) == 0x6F) {
                        return;
                    }
                    if (*(u8 *)(state + 0x16) != 0) {
                        func_002C5A48((void *)0);
                    }
                    if (*(u8 *)(state + 0x17) != 0) {
                        func_0036E9D0(s2 + 0x80, s2 + 0x90);
                    }
                }
                *(u8 *)(arg0 + 0x20) = 3;
            }
        }
    } else {
        s32 i = *(s32 *)(state + 0xC);
        char *base = D_L20_001B2F9C;
        u8 a1 = *(u8 *)(state + 0x14);
        u8 v1 = *(u8 *)(base + (i << 8) + 0x20);
        u16 h = *(u16 *)(arg0 + 0xB2);

        if (v1 != a1) {
            if (*(u8 *)(state + 0x15) == 0) {
                return;
            }
            if (func_0033E7F8(arg0) == 0) {
                return;
            }
            h = *(u16 *)(arg0 + 0xB2);
        }
        if (h & 1) {
            s32 idx = (h << 16) >> 17;
            s32 g = D_001A79F0_1;
            u8 *p = (u8 *)((s32)D_L20_0019B4A8 + idx + (g << 10));
            u8 b = *p;

            if (((b >> 4) & 7) != 7) {
                *p = b + 0x10;
            }
        } else {
            s32 idx = (h << 16) >> 17;
            s32 g = D_001A79F0_1;
            u8 *p = (u8 *)((s32)D_L20_0019B4A8 + idx + (g << 10));
            u8 b = *p;

            if ((b & 7) != 7) {
                *p = b + 1;
            }
        }
        if (*(u8 *)(state + 0x17) != 0) {
            func_00327008(*(u8 *)(arg0 + 0xB0));
        }
        *(u8 *)(arg0 + 0x20) = 2;
        *(s32 *)(state + 0x4) = 0x78;
        {
            f32 x = func_003033B8(0x78);
            s32 w = *(s32 *)(state + 0x10);
            f32 f1 = 1.0f / x;

            func_004604E8(w);
            *(f32 *)(state + 0x8) = f1;
        }
    }
}

/* ------------------------------------------------------------------
   Wave 21, worker 08.  What this wave measured on the same body.

   The attempt below is 616 bytes against the retail's 648 and its
   instructions are the retail's; what is wrong is the dispatch and
   with it the order of the blocks.  The retail's dispatch, measured:

     lbu  $v1,0x20($s0)        <- the byte, not the word
     beq  $v1,$v0(=1),.L003C0140   <- v == 1, out of line
     lw   $s1,0x68($s0)        <- the delay slot: the state
     slti $v0,$v1,0x2
     beql $v0,$zero,.L003C008C  <- v >= 2, out of line, branch likely
     addiu $v0,$zero,0x2       <- the delay slot: the constant 2
     beql $v1,$zero,.L003C009C  <- v == 0, out of line, branch likely
     lw   $v0,0xC($s1)         <- the delay slot
     b    .L003C01B0           <- the return, the fall through
     ld   $s0,0x0($sp)         <- the delay slot

   Two things the attempt gets wrong and that are now measured:

   - the attempt reads the word at 0x20 (`lw $v1,0x20($s0)`) and
     compares it with a constant in $a2; the retail reads the BYTE
     (`lbu`) and keeps the constant 1 in $v0.  So the source's
     variable is `s32 v = *(u8 *)(arg0 + 0x20);`: the load is the
     zero-extending byte load and the comparison with 2 is the signed
     `slti` (a signed int against 2), which is why the retail has
     `slti` and not `sltiu`.
   - all three cases are OUT OF LINE in the retail (0x3C0140, 0x3C008C
     and 0x3C009C) and the dispatch falls through to the return at
     0x3C01B0; the attempt keeps the v == 0 case in the fall through.
     The source's shape is therefore

       if (v == 1) { goto case1; }
       else if (v >= 2) { goto case2; }
       else if (v == 0) { goto case0; }

     with the case bodies laid out in the order the retail has them
     (case 2 at 0x3C008C, case 0 at 0x3C009C, case 1 at 0x3C0140 and
     the return at 0x3C01B0).  The two branch-likely forms (the
     `beql`s) are what this cc1 emits when the block ends in an
     unconditional jump to the body (see the note on
     func_L20_0039D680).

   The attempt below is unchanged; it is the closest one.
*/

/* ------------------------------------------------------------------
   Wave 21, worker 08.  A new attempt, much closer than the one above.

   The attempt below is 636 bytes against the retail's 648 and its
   dispatch is the retail's, instruction for instruction:

     lbu  $v1,0x20($s0)        <- the byte, not the word
     beq  $v1,$v0(=1),.L003C0140
     lw   $s1,0x68($s0)        <- the state
     slti $v0,$v1,0x2
     beql $v0,$zero,.L003C008C
     addiu $v0,$zero,0x2
     beql $v1,$zero,.L003C009C
     lw   $v0,0xC($s1)
     b    .L003C02C4
     ld   $s0,0x0($sp)

   What this wave measured and fixed against the retail:

   - the variable is `s32 v = *(u8 *)(arg0 + 0x20);`: the retail reads
     the BYTE (`lbu`, the attempt above read the word) and compares it
     with the signed constant 2 (`slti`, not `sltiu`), because the byte
     is zero-extended into a signed int.
   - all three cases are out of line and the dispatch falls through to
     the return, so the source's shape is

       if (v == 1) goto case1;
       if (v >= 2) goto case2;
       if (v == 0) goto case0;
       goto end;

     and the case bodies are laid out in the order the retail has them
     (the case 2 test at 0x3C008C, the case 0 block at 0x3C009C, the
     case 1 block at 0x3C0140, the case 2 body at 0x3C0254 and the
     return at 0x3C02C4).
   - the "v == 2" test is inside the case 2 block (the constant 2 is
     materialised in the dispatch's delay slot and compared there), so
     the case 2 block is "if (v == 2) { the body } else { the return }".
   - the case 0 block tests the three fields of the state separately
     (the retail has three branches: `bltz` on state[0xC], `bltz` on
     state[0] and `bgezl` on state[0x10]), and the second test of the
     table is `if (h & 1) b >>= 4;` before `if ((b & 7) != 0)`: the
     attempt above was missing that shift (three instructions).
   - the case 1 block stores the result of the division before the
     call to f_004604E8 (`*(f32 *)(state + 0x8) = 1.0f /
     func_003033B8(0x78); func_004604E8(*(s32 *)(state + 0x10));`), so
     the division's result is not live across the call and the retail
     saves no floating point register (the frame is 0x20, the attempt
     above had 0x30 because the result was in a variable).
   - the case 1 block reads the index at 0xB2(arg0) in both arms of
     the "v1 == a1" test (the retail has one load in the delay slot of
     the branch and one after it).

   What is still different:

   - the register allocation of the two pointers: the retail gives $s0
     to the copy of arg0 (`daddu $s0,$a0,$zero`) and $s1 to the state,
     the attempt gives $s0 to the state and $s1 to the copy.  Giving
     the copy its own variable (`char *self = arg0;`) does not change
     it.
   - the constant 1 of the dispatch's first test is in $v0 in the
     retail and in $a3 in the attempt.
   - three instructions are still missing (636 bytes against 648).

   The attempt below is the closest one of this wave.
*/

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

extern void func_003253A8(void *);
extern int func_0033E7F8(void *);
extern int func_00327008(u8);
extern float func_003033B8(int);
extern float func_00301ED0(void *);
extern void func_003C0350(void *);
extern void func_002C5A48(void *);
extern void func_0036E9D0(void *, void *);
extern void func_003C02D0(void *);
extern void func_004604E8(s32);

extern char *D_L20_001B2F9C __attribute__((nosda));
extern char D_L20_0019B4A8[] __attribute__((nosda));
extern char D_L20_0021EFB0[] __attribute__((nosda));
extern char D_L20_00189E20[] __attribute__((nosda));
extern int D_001A79F0;
extern int D_001A79F0_1 __attribute__((sda));

void func_L20_003C0048(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    s32 v = *(u8 *)(arg0 + 0x20);

    if (v == 1) {
        goto case1;
    }
    if (v >= 2) {
        goto case2;
    }
    if (v == 0) {
        goto case0;
    }
    goto end;

case2:
    if (v == 2) {
        goto case2body;
    }
    goto end;

case0:
    if (*(s32 *)(state + 0xC) < 0) {
        goto err0;
    }
    if (*(s32 *)(state + 0x0) < 0) {
        goto err0;
    }
    if (*(s32 *)(state + 0x10) >= 0) {
        goto body0;
    }
err0:
    func_003253A8(arg0);
    goto end;
body0:
    {
        u16 a1 = *(u16 *)(arg0 + 0xB2);
        u8 c = *(u8 *)((s32)D_L20_0021EFB0 + (s16)a1);

        if (c != 0) {
            goto set3;
        }
        {
            s32 g = D_001A79F0;
            u8 b = *(u8 *)((s32)D_L20_0019B4A8 + ((a1 << 16) >> 17) + (g << 10));

            if (a1 & 1) {
                b = b >> 4;
            }
            if ((b & 7) != 0) {
                goto set3;
            }
        }
        *(u8 *)(arg0 + 0x20) = 1;
        func_003C02D0(arg0);
    }
    goto end;

case1:
    {
        s32 i = *(s32 *)(state + 0xC);
        char *base = D_L20_001B2F9C;
        u8 a1 = *(u8 *)(state + 0x14);
        u8 v1 = *(u8 *)(base + (i << 8) + 0x20);
        u16 h;

        if (v1 == a1) {
            h = *(u16 *)(arg0 + 0xB2);
        } else {
            if (*(u8 *)(state + 0x15) == 0) {
                goto end;
            }
            if (func_0033E7F8(arg0) == 0) {
                goto end;
            }
            h = *(u16 *)(arg0 + 0xB2);
        }
        if (h & 1) {
            s32 idx = (h << 16) >> 17;
            s32 g = D_001A79F0_1;
            u8 *p = (u8 *)((s32)D_L20_0019B4A8 + idx + (g << 10));
            u8 b = *p;

            if (((b >> 4) & 7) != 7) {
                *p = b + 0x10;
            }
        } else {
            s32 idx = (h << 16) >> 17;
            s32 g = D_001A79F0_1;
            u8 *p = (u8 *)((s32)D_L20_0019B4A8 + idx + (g << 10));
            u8 b = *p;

            if ((b & 7) != 7) {
                *p = b + 1;
            }
        }
        if (*(u8 *)(state + 0x17) != 0) {
            func_00327008(*(u8 *)(arg0 + 0xB0));
        }
        *(u8 *)(arg0 + 0x20) = 2;
        *(s32 *)(state + 0x4) = 0x78;
        *(f32 *)(state + 0x8) = 1.0f / func_003033B8(0x78);
        func_004604E8(*(s32 *)(state + 0x10));
    }
    goto end;

case2body:
    {
        func_00301ED0(state + 0x4);
        func_003C0350(arg0);
        if (*(s32 *)(state + 0x4) != 0) {
            goto end;
        }
        {
            char *s2 = (char *)D_L20_00189E20;

            if (*(s32 *)(s2 + 0x2294) == 0x6F) {
                goto end;
            }
            if (*(u8 *)(state + 0x16) != 0) {
                func_002C5A48((void *)0);
            }
            if (*(u8 *)(state + 0x17) != 0) {
                func_0036E9D0(s2 + 0x80, s2 + 0x90);
            }
        }
set3:
        *(u8 *)(arg0 + 0x20) = 3;
    }
    goto end;

end:
    return;
}
