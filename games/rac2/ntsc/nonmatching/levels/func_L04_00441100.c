/* wave 6 worker w38 - src/levels/04_barlow/L04_00422A00.c
   Best attempts for the four functions left as INCLUDE_ASM.
   Each entry names the last check.py line observed. */

/* ------------------------------------------------------------------ */
/* func_L04_00422D40 (104 o) - 96 o produced. "04_barlow: 96 bytes, expected 104"
   Misses exactly two instructions: the retail writes a 128-bit zero to the
   stack slot with a materialised register (por/psubb $a0,$zero,$zero + sq $a0)
   before the four swc1's overwrite it, and it materialises the destination
   pointer add (addiu $v1,$v1,0x30 + sq $v0,0($v1)) where cc1 folds the 0x30
   into the store offset. All `= 0` / `{0}` spellings tried fold to `sq $zero`;
   the fold pass in the chain is what removes the register form.
   Measured literals: 80000.0f, -70000.0f, 140000.0f, 1.0f; D_L04_001B9984 = 0.
   Last standing form: */

typedef int TI128 __attribute__((mode(TI)));

typedef union {
    float f[4];
    TI128 q;
} Quad_u;

void func_L04_00422D40(char *a0) {
    char *p = *(char **)(a0 + 0x68) + 0x30;
    Quad_u v;

    v.q = 0;
    D_L04_001B9984 = 0;
    v.f[0] = 80000.0f;
    v.f[1] = -70000.0f;
    v.f[2] = 140000.0f;
    v.f[3] = 1.0f;
    *(TI128 *)p = v.q;
}

/* ------------------------------------------------------------------ */
/* func_L04_00441100 (116 o) - right size, one allocation away.
   "04_barlow: instruction at +0x20 differs"
   Produced: addiu $a0,$zero,-0x1 / lw $v1,0x0($v0) / beq $v1,$a0
   Retail  : addiu $v1,$zero,-0x1 / lw $a1,0x0($v0) / beq $a1,$v1
   Every spelling tried (inline double deref, u32 compare, named s32 temp,
   reversed operand order, nested positive if) keeps the constant in $a0. */

/* ------------------------------------------------------------------ */
/* func_L04_00422D40 (104 o) - 96 o produced. "04_barlow: 96 bytes, expected 104"
   Misses exactly two instructions: the retail writes a 128-bit zero to the
   stack slot with a materialised register (por/psubb $a0,$zero,$zero + sq $a0)
   before the four swc1's overwrite it, and it materialises the destination
   pointer add (addiu $v1,$v1,0x30 + sq $v0,0($v1)) where cc1 folds the 0x30
   into the store offset. All `= 0` / `{0}` spellings tried fold to `sq $zero`;
   the fold pass in the chain is what removes the register form.
   Measured literals: 80000.0f, -70000.0f, 140000.0f, 1.0f; D_L04_001B9984 = 0.
   Last standing form: */

typedef int TI128 __attribute__((mode(TI)));

typedef union {
    float f[4];
    TI128 q;
} Quad_u;

void func_L04_00422D40(char *a0) {
    char *p = *(char **)(a0 + 0x68) + 0x30;
    Quad_u v;

    v.q = 0;
    D_L04_001B9984 = 0;
    v.f[0] = 80000.0f;
    v.f[1] = -70000.0f;
    v.f[2] = 140000.0f;
    v.f[3] = 1.0f;
    *(TI128 *)p = v.q;
}

/* ------------------------------------------------------------------ */
/* func_L04_00441100 (116 o) - right size, one allocation away.
   "04_barlow: instruction at +0x20 differs"
   Produced: addiu $a0,$zero,-0x1 / lw $v1,0x0($v0) / beq $v1,$a0
   Retail  : addiu $v1,$zero,-0x1 / lw $a1,0x0($v0) / beq $a1,$v1
   Every spelling tried (inline double deref, u32 compare, named s32 temp,
   reversed operand order, nested positive if) keeps the constant in $a0. */

void func_L04_00441100(unsigned char *a0) {
    if (a0[0x20] != 0)
        return;
    if (**(signed int **)(a0 + 0x68) == -1)
        return;
    if (func_342228(&D_L04_00189EA0) == 0)
        return;
    func_2CC1F0(&D_L04_002CA4D0);
    D_L04_001395C8 = 1;
    func_33B9B0(a0, 1, -1);
}

/* ------------------------------------------------------------------ */
/* func_L04_00439870 (224 o) - 264 o produced. LICM wall.
   The three `D_L04_001B1E00/04/08 = 2.75f` stores are the problem: cc1 hoists
   their `lui` addresses into callee-saved registers ($s4..$fp) out of the loop,
   while the retail re-materialises `lui $at,0x1B` at each store. The rest of
   the body (guard, counted loop, func_3325E0 call, f20 = 2.75f hoisted) is
   shape-identical. */

void func_L04_00439870(unsigned char *a0) {
    unsigned char *s2;
    signed int i;

    if (D_L04_001A8F00 != 2)
        return;
    if (D_L04_001A7A40 == 0)
        return;
    s2 = &D_L04_001BFD80;
    for (i = 0; i < *(short *)(s2 + 0x44); i++) {
        unsigned char *p = *(unsigned char **)(s2 + 0x18C + i * 4);

        if (*(short *)(p + 0xAA) == *(short *)(a0 + 0xAA)) {
            if (D_L04_001B1DE1 == 0) {
                func_3325E0(p, 0, &D_L04_001B1DE0);
                D_L04_001B1E00 = 2.75f;
                D_L04_001B1E04 = 2.75f;
                D_L04_001B1E08 = 2.75f;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* func_L04_00455520 (220 o) - right size, one allocation away.
   "04_barlow: instruction at +0x1c differs"
   Produced: lw $a2,0x68($s1) ... lw $v0,0x0($a2) / lui $a0 / addu $a0,$v0,$a0
   Retail  : lw $a1,0x68($s1) ... lw $a0,0x0($a1) / lui $v0 / addu $a0,$v0,$a0
   Every other instruction of the 55 is identical. D_L04_001B24DC must be a
   POINTER (retail `lui $v0,0x1B ; lw $v0,0x24DC($v0)`), not an array. */

void func_L04_00455520(unsigned char *a0) {
    unsigned char *s0;
    unsigned char *p = *(unsigned char **)(a0 + 0x68);

    if (a0[0x20] == 0) {
        if ((&D_L04_0019B2E8)[a0[0xB0] + D_L04_001A79F0 * 16] & 0x80) {
            func_3317E8(a0);
            return;
        }
        a0[0x20] = 1;
    }
    if (a0[0x20] != 1)
        return;
    if (func_34C4C8(D_L04_001B24DC + (*(unsigned int *)p << 8)) == 0)
        return;
    func_333408(a0[0xB0]);
    s0 = &D_L04_001BF880;
    func_314E50(s0, 0x38);
    func_314F98(s0);
    func_394268();
    func_3317E8(a0);
}
