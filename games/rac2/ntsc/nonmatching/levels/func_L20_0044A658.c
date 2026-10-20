/* Near miss from src/levels/20_yeedil/L20_00449178-g8.c: 452 bytes, retail 480.
   The attempt belongs in the -G8 unit: the body reaches D_L20_001B2418 through
   $gp and D_00189E20 absolutely (nosda).

   Everything but the two 16-byte copies is the retail instruction: the
   prologue, the two branches on arg0[0x20], the func_002A7750 /
   func_002A9640 / func_002A96A0 / func_002AA2E8 / func_002B3BA8 calls with
   their argument setup, the bc1fl on the state's float, the 0x12F2/0x12EF
   test, the state stores and the epilogue all match.

   The copies are the wall (the same one as func_L20_0044A838 and
   func_L20_00449FD0 in this level): the retail materialises both addresses
       addiu $a1, $s1, 0x180 ; addiu $a0, $s0, 0x10 ;
       lq $v0, 0x0($a0) ; sq $v0, 0x0($a1)
   while this cc1 folds the constant offsets into the operands
       lq $v0, 0x10($s0) ; sq $v0, 0x180($s1)
   (8 bytes short for that copy).  The second copy, out of D_00189E20, is
   the same: the retail keeps the symbol in a register and adds 0x2290 and
   0xD0 to it, this cc1 folds 0xD0 into the lq.  Spelling tried for the
   symbol: `u32 *base = (u32 *)D_00189E20;` with `base[0x8A4]` and
   `*(TI128 *)((char *)base + 0xD0)` - that puts the symbol in a register
   and makes the load match the retail exactly, but the copy's offset
   still folds.

   With the copies unfolded the register allocation follows the retail's:
   the retail spends $a0/$a1 on the copy's addresses, so it keeps the
   flags in $v1 (`lhu $v1,0x34($s0)`) and the -1 in $v0, while this cc1
   (with $a0/$a1 free) keeps the flags in $v0 and the -1 in $a0.

   Measured this wave (2026-10-20): the fold is in the address printer -
   the TI access keeps the plus (GO_IF_LEGITIMATE_ADDRESS accepts
   plus(reg, const_int) for TImode when offset + 16 - 1 <= 32767) and the
   offset is then folded into the lq/sq.  Spellings tried and measured: a
   TI128 pointer variable, a char * variable, a u32/u64 address variable, a
   struct with a TI128 member, a union with a float array, the pointer used
   twice, the pointer live across a call, the pointer taken from a struct
   field, the offsets applied one at a time, a 32-bit (int) address, a
   64-bit address (keeps the copy in a register but adds a dsll/dsra pair
   and uses daddu, not addiu), __builtin_memcpy(...,16), and the per-file
   flags -g, -mno-split-addresses, -fno-schedule-insns, -fno-schedule-insns2
   (none of them changes the fold).  A pointer that is a plain parameter or
   a plain load keeps the retail's shape, so the wall is exactly the
   source-level constant offset. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));


/* func_L20_0044A658 reaches D_L20_001B2418 through $gp and D_00189E20 absolutely,
   so it lives in this -G8 unit. */

extern float D_L20_001B2418 __attribute__((sda));

extern u32 D_00189E20[] __attribute__((nosda));

extern void func_002A7750(char *, int, int);

extern void *func_002A9640(char *, int, int);

extern float func_002A96A0(char *, char *);


extern void func_002B3BA8(char *, u32, int, int, float, float);

extern void func_L20_0044AB68(char *);

void func_L20_0044A658(char *arg0) {
    char *state;
    char *p;
    u32 *q;
    short t;
    float f;

    state = *(char **)(arg0 + 0x68);
    if (*(u8 *)(arg0 + 0x20) == 0) {
        func_002A7750(arg0, 1, -1);
        *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
        *(int *)(arg0 + 0x98) = 0;
        *(TI128 *)(state + 0x180) = *(TI128 *)(arg0 + 0x10);
        *(float *)(arg0 + 0x2C) =
            *(float *)(*(u32 *)(arg0 + 0x24) + 0x24) * D_L20_001B2418;
        *(int *)(state + 0x1B8) = -1;
    }
    if (*(u8 *)(arg0 + 0x20) == 1) {
        return;
    }
    p = func_002A9640(arg0, 0x10000, 0);
    if (p != 0) {
        q = *(u32 **)(p + 0x20);
        if (q != 0) {
            t = *(short *)((char *)q + 0xAA);
            if (t == 0x12F2 || t == 0x12EF) {
                p = 0;
            }
        }
    }
    f = func_002A96A0(arg0, p);
    func_002AA2E8((float *)(state + 0x20), 0.0f, f);
    *(u8 *)(arg0 + 0xA8) = 0xFF;
    if (*(float *)(state + 0x20) == 0.0f) {
        func_L20_0044AB68(arg0);
        *(u8 *)(arg0 + 0xA8) = 0xFF;
        func_002A7750(arg0, 1, -1);
        *(int *)(arg0 + 0x98) = 0;
        *(u16 *)(arg0 + 0x34) = (*(u16 *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    }
    if (*(u8 *)(arg0 + 0x20) == 5) {
        return;
    }
    p = *(char **)(state + 0x1B0);
    if (p != 0) {
        *(u32 *)(state + 0x150) = (u32)p;
        *(TI128 *)(state + 0x140) = *(TI128 *)(p + 0x10);
        *(float *)(state + 0x148) = *(float *)(state + 0x148) + 0.5f;
        return;
    }
    func_002B3BA8(arg0, *(u32 *)(state + 0x160), 0, 0, 1024.0f, 37.0f);
    if (*(u32 *)(state + 0x150) != 0) {
        return;
    }
    {
        u32 *base = (u32 *)D_00189E20;
        *(u32 *)(state + 0x150) = base[0x8A4];
        *(TI128 *)(state + 0x140) = *(TI128 *)((char *)base + 0xD0);
    }
}

/* WAVE 3 (2026-10-20, worker 09, unit src/levels/20_yeedil/L20_00449178-g8.c).
   The attempt above is unchanged and is still the best one.  Measured
   size: 452 bytes against the retail's 480 - twenty-eight bytes short.
   The retail materialises BOTH addresses of the copy
       addiu $a1, $s1, 0x180 ; addiu $a0, $s0, 0x10 ;
       lq $v0, 0x0($a0) ; sq $v0, 0x0($a1)
   where this cc1 folds them into the operands
       lq $v0, 0x10($s0) ; sq $v0, 0x180($s1)
   and the same for the second copy out of D_00189E20.  The mechanism (the
   CSE's find_best_addr cost tie) is written up in full in
   nonmatching/levels/func_L20_0044A838.c (wave 3); only a SYMBOL base
   keeps the address in a register, and the symbol here is reached through
   a variable, so the offset folds.

   With the copies unfolded the register allocation follows the retail's
   (the retail spends $a0/$a1 on the copy's addresses, so it keeps the
   flags in $v1 and the -1 in $v0), which is why the flags move here. */
