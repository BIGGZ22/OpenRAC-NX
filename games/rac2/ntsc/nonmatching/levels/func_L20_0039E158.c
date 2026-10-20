#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

/* 128-bit copies reach lq/sq only through TImode. */
typedef int TI128 __attribute__((mode(TI)));

extern void func_002B3BA8(char *, int, float, float, int, int);

extern char *func_L20_004448A8(char *, char *);

/* Near miss from src/levels/20_yeedil/L20_0039C448.c: 140 bytes; 132-byte candidate; 0/1.
    (Replaces an earlier draft from commit adcbd51, which used the inline-assembly `qcopy`
    helper that AGENTS.md's "Plain C" rule forbids; that draft is in that commit.)
    Everything is the retail's: the frame (0x20), the two calls with their argument
    registers (the float pair in $f12/$f13 comes before the last int argument, so the
    `mov.s $f13,$f12` sits before the `jal` and the `daddu $a3,$zero,$zero` in its delay
    slot), the `bnez` with the store of the pointer in its delay slot, the reload of
    +0x280 after the two stores and the store of it at +0x200.
    The only difference left is the 128-bit copy: the retail materialises both addresses
    (`addiu $a1,$s0,0x1F0; addiu $a0,$v1,0x10; lq $v0,0($a0); sq $v0,0($a1)`) while this
    cc1 folds each constant offset into the `lq`/`sq` (`lq $v0,0x10($v1)`,
    `sq $v0,0x1F0($s0)`).  That is two instructions, the eight bytes of difference.
    Measured in this wave with about thirty spellings, all of them fold: a TI128 pointer
    variable (one and two of them), a struct member, an array index, a `volatile` load,
    two C names for the same pointer, a pointer used twice and a pointer kept across a
    call, an integer cast (signed and unsigned) and a 64-bit address (which keeps the
    address in a register but costs `daddu` plus `dsll`/`dsra`).
    The rule that holds: this cc1 folds a constant offset into the `lq`/`sq` whenever the
    address is a `plus` of a register; the address only stays in a register when it comes
    from something that is not a `plus` (a load, a call result) or when a mode conversion
    forces it (which then costs extra instructions).  The retail does both: it folds the
    offset in func_L08_003391C0 (`lq $v1,0x10($a0)`) and in func_L20_0043F510
    (`lq $v0,0x10($s0)`), and it materialises it here and in func_L20_0039F210,
    func_L20_0039DCF8 and func_L20_004395B0.  Same wall as the other TI copies in this
    level. */
void func_L20_0039E158(char *arg0) {
    char *s1 = arg0;
    char *s0 = *(char **)(s1 + 0x68);
    char *v;
    int p;

    if (*(int *)(s0 + 0x27C) != 0) {
        *(int *)(s0 + 0x280) = 0;
        func_002B3BA8(arg0, *(int *)(s0 + 0x230), 0.0f, 0.0f, 0, 0);
    } else {
        v = func_L20_004448A8(*(char **)(s1 + 0xB8), s1);
        *(int *)(s0 + 0x280) = (int)v;
        if (v == 0) {
            *(int *)(s0 + 0x280) = *(int *)(s1 + 0xB8);
        }
        p = *(int *)(s0 + 0x280);
        *(int *)(s0 + 0x200) = p;
        *(TI128 *)(s0 + 0x1F0) = *(TI128 *)((char *)p + 0x10);
    }
}
