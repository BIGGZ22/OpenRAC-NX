/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 592 bytes, retail 624.
   The attempt belongs in the -G8 unit: the body reaches D_001A79F0 through
   $gp and D_0019B2E8 / D_00189EA0 absolutely (nosda).

   The state machine is the retail's shape: the switch on arg0[0x20] with
   its two branch-likely tests, the two table lookups
   (D_0019B2E8 + arg0[0xB0] + D_001A79F0 * 16, the `sll` by 4 and the
   0x80 mask), the func_0033E7F8 / func_0033E808 calls, the pi/4 clamp of
   the case 2, the state stores and the epilogue all match.

   The difference is the four 16-byte copies (the wall of this level, see
   func_L20_0044A838 and func_L20_00449FD0): the retail materialises both
   addresses of every copy into registers (`lq $v0,0x0($a0) ;
   sq $v0,0x0($sp)` with $a0 = arg0+0x10 computed by an addiu) and this
   cc1 folds the constant offsets into the operands.  That is 32 bytes
   short, and it also shifts the register allocation (the retail keeps the
   state, the state+0x180 pointer and arg0+0x10 in callee-saved
   registers, so its frame is 0x60 where this cc1's is 0x50). */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));


extern s32 D_001A79F0 __attribute__((sda));

extern u8 D_0019B2E8[] __attribute__((nosda));

extern u32 D_00189EA0[] __attribute__((nosda));

extern void *func_0033E808(u32 *);

extern int func_0033E7F8(char *);

extern void func_00336648(char *, char *, char *, char *);

void func_L20_00449338(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    TI128 v0, v1, v2;
    u32 a1;
    float f0, f1, f2, f3;

    v0 = *(TI128 *)(arg0 + 0x10);
    v1 = *(TI128 *)(arg0 + 0xF0);
    switch (*(u8 *)(arg0 + 0x20)) {
    case 0:
        *(TI128 *)(state + 0x60) = v0;
        *(TI128 *)(state + 0x70) = v1;
        a1 = *(u32 *)(state + 0x88);
        if (*(u8 *)(arg0 + 0xB0) != 0xFF) {
            if (*(u32 *)(state + 0x8C) != 0) {
                if (*(u8 *)((char *)D_0019B2E8 + *(u8 *)(arg0 + 0xB0) +
                            D_001A79F0 * 16) & 0x80) {
                    goto l2c;
                }
            } else {
                a1 = *(u32 *)(state + 0x88);
            }
        }
        if (a1 == (u32)-1) {
            func_002A7750(arg0, 1, -1);
            break;
        }
        if (func_0033E808(D_00189EA0) == 0) {
            func_002A7750(arg0, 1, -1);
            break;
        }
    l2c:
        f0 = *(float *)(state + 0x68);
        f1 = *(float *)(state + 0x80);
        *(u16 *)(arg0 + 0x34) |= 2;
        *(float *)(arg0 + 0x18) = f0 + f1;
        func_002A7750(arg0, 3, -1);
        break;
    case 1:
        if (*(u8 *)(arg0 + 0xBC) != 0) {
            func_002A7750(arg0, 2, -1);
            break;
        }
        if (func_0033E7F8(arg0) != 0) {
            func_002A7750(arg0, 2, -1);
            break;
        }
        a1 = *(u32 *)(state + 0x88);
        if (*(u8 *)(arg0 + 0xB0) != 0xFF) {
            if (*(u32 *)(state + 0x8C) != 0) {
                if (*(u8 *)((char *)D_0019B2E8 + *(u8 *)(arg0 + 0xB0) +
                            D_001A79F0 * 16) & 0x80) {
                    func_002A7750(arg0, 2, -1);
                    break;
                }
            } else {
                a1 = *(u32 *)(state + 0x88);
            }
        }
        if (a1 == (u32)-1) {
            break;
        }
        if (func_0033E808(D_00189EA0) != 0) {
            func_002A7750(arg0, 2, -1);
        }
        break;
    case 2:
        f0 = *(float *)(state + 0x84);
        f3 = 1.0f;
        f1 = *(float *)(state + 0x90);
        f0 = f3 / f0;
        f1 = f1 + f0;
        *(float *)(state + 0x90) = f1;
        if (f1 <= f3) {
            *(float *)(state + 0x90) = f3;
        }
        f1 = *(float *)(state + 0x90);
        f2 = *(float *)(state + 0x80);
        f0 = *(float *)(state + 0x68);
        f1 = f1 * f2;
        f0 = f0 + f1;
        *(float *)(arg0 + 0x18) = f0;
        if (*(float *)(state + 0x90) <= f3) {
            func_002A7750(arg0, 3, -1);
        }
        break;
    }
    if (*(u8 *)(arg0 + 0x20) < 3) {
        func_00282CF0(&v2, arg0 + 0x10, &v0);
        func_00336648(state + 0x20, &v2, &v1, arg0 + 0xF0);
    }
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one.  Measured
   size: 592 bytes against the retail's 624 - thirty-two bytes short,
   which is exactly the retail's four copies, two instructions each:
       addiu $a0, $s1, 0x10 ; ... ; lq $v0, 0x0($a0) ; sq $v0, 0x0($sp)
   where this cc1 folds the offsets into the operands.  The mechanism (the
   CSE's find_best_addr cost tie) is written up in full in
   nonmatching/levels/func_L20_0044A838.c (wave 3).

   The frame difference (0x50 here, 0x60 in the retail) and the extra
   callee-saved registers follow from the copies: the retail keeps the
   state pointer, the state+0x180 pointer and arg0+0x10 in registers. */
