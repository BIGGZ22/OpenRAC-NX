/* func_L20_00442890 (352 bytes, level 20): near miss, 86 of 88 words differ.
 *
 * The whole structure is recovered and every call is retail's; the residues are the
 * register allocation and the instruction order.
 *
 * What the retail body does (from build/20_yeedil/asm/5B0B0.s):
 *  - the base D_L20_001B9E80 is materialised into $s1 (`lui $v0,%hi(D_1B9E80);
 *    addiu $s1,$v0,%lo(D_1B9E80)` - the high part in a scratch, the low part in $s1);
 *  - `lw $v1,0x190($s1)` is the parent pointer, `lh $v0,0x86($v1)` its flag: if it is set
 *    the function returns;
 *  - three calls into the shared level unit with the $gp floats D_L20_001B2158 (gp+0x3168),
 *    D_L20_001B215C (gp+0x316C), D_L20_001B2160 (gp+0x3170) and the constants 0.003f and
 *    0.005f: func_L00_00424A08(0, D_001B2158, 0.003f), func_L00_00424A50(D_001B215C,
 *    0.003f) (that one takes no int argument), func_L00_00424A88(0, D_001B2160, 0.005f);
 *  - if arg0[0x95] is 4 or 2 the function returns;
 *  - `func_00283248(arg0[0x10] - D_189E20[0x20], arg0[0x14] - D_189E20[0x21])` (the boot
 *    table read through a pointer in a register, the two floats at +0x80 and +0x84);
 *  - the state (arg0[0x68]) is loaded, the parent becomes arg0 = parent[0x70] + 0x1C0, and
 *    `func_00283248(*(float *)(arg0+0x160), *(float *)(arg0+0x164))` follows;
 *  - `func_00283B98(that, 3.1415927f)` then `func_00283BE0(first, that)`;
 *  - `func_002AAA00` is called with (the result of the blend, state+0x17F0, 0, 0.0f,
 *    D_L20_001B2150 * 4.84813745e-06f, the same, D_L20_001B2154 * 0.000290888245f);
 *  - arg0[0x1C] = 1.0f and arg0[0x14] = -state[0x17F0].
 *
 * The residues:
 *  1. the base address: retail puts the high part in a scratch ($v0) and the low part in
 *     $s1, this cc1 puts both in $s1 (`lui $s1,%hi(D); addiu $s1,$s1,%lo(D)`), and it
 *     emits the pair after the first save where retail interleaves it with the saves;
 *  2. the state pointer reuses $s1 in retail (the base dies, the state takes its register)
 *     and $s0 in this cc1;
 *  3. the second float pair: retail computes `addiu $v0,$s0,0x160` and reads the second
 *     float at 0x4($v0), this cc1 reads it at 0x164($s0) (a float* local does not force
 *     the address into a register here);
 *  4. the order of the constant materialisations (3.1415927f, 4.84813745e-06f,
 *     0.000290888245f) and of the last five instructions.
 *
 * The unit has to be the -G8 sibling (five $gp floats). With -mno-split-addresses the body
 * is 344 bytes, with the default flags 340 against retail's 352.
 */
#include "common.h"

typedef float f32;

extern char D_L20_001B9E80[];

extern float D_189E20[];

extern float D_L20_001B2150 __attribute__((sda));

extern float D_L20_001B2154 __attribute__((sda));

extern float D_L20_001B2158 __attribute__((sda));

extern float D_L20_001B215C __attribute__((sda));

extern float D_L20_001B2160 __attribute__((sda));

extern void func_L00_00424A08(int, float, float);

extern void func_L00_00424A50(float, float);

extern void func_L00_00424A88(int, float, float);

extern float func_00283248(float, float);

extern float func_00283B98(float, float);

extern float func_00283BE0(float, float);

extern int func_002AAA00(char *, char *, int, float, float, float, float);

void func_L20_00442890(char *arg0)
{
    char *s0 = arg0;
    char *s1 = (char *)D_L20_001B9E80;
    char *v1 = *(char **)(s1 + 0x190);
    float *p;
    float r, q, w;

    if (*(short *)(v1 + 0x86) != 0) {
        return;
    }
    func_L00_00424A08(0, D_L20_001B2158, 0.003f);
    func_L00_00424A50(D_L20_001B215C, 0.003f);
    func_L00_00424A88(0, D_L20_001B2160, 0.005f);
    if (*(unsigned char *)(s0 + 0x95) == 4 ||
        *(unsigned char *)(s0 + 0x95) == 2) {
        return;
    }
    r = func_00283248(*(float *)((char *)D_189E20 + 0x80) - *(float *)(s0 + 0x10),
                      *(float *)((char *)D_189E20 + 0x84) - *(float *)(s0 + 0x14));
    s1 = *(char **)(s0 + 0x68);
    s0 = *(char **)(v1 + 0x70) + 0x1C0;
    p = (float *)(s0 + 0x160);
    q = func_00283248(p[0], p[1]);
    q = func_00283B98(q, 3.1415927f);
    w = func_00283BE0(r, q);
    func_002AAA00((char *)&w, s1 + 0x17F0, 0, 0.0f,
                   D_L20_001B2150 * 4.84813745e-06f,
                   D_L20_001B2150 * 4.84813745e-06f,
                   D_L20_001B2154 * 0.000290888245f);
    *(float *)(s0 + 0x1C) = 1.0f;
    *(float *)(s0 + 0x14) = -*(float *)(s1 + 0x17F0);
}

/* ---- wave 3 (20_yeedil, 2026-10-20, worker 02): the base address, measured -------
 *
 * The residue 1 is the split address form, and the two profiles each give the OTHER
 * form, so no per-file flag reaches the retail's:
 *
 *   default profile      lui $s1,%hi(D_1B9E80); addiu $s1,$s1,%lo(D_1B9E80)
 *                        - the high part and the address in the SAME register (the
 *                        base is used once, so the compiler materialises it once);
 *   -mno-split-addresses lui $s1,%hi(D_1B9E80); lw $s1,0x190($s1)
 *                        - the assembler expands the plain symbol load into the
 *                        "base == dest" pair, again one register;
 *   retail               lui $v0,%hi(D_1B9E80); addiu $s1,$v0,%lo(D_1B9E80)
 *                        - the high part in a scratch, the address in $s1: the SPLIT
 *                        form, with no reason for the compiler to keep the high part
 *                        alive (the base is used once).
 *
 * The same split form is what func_L20_004431E0 wants for its table base, and the
 * same flag gives the opposite there.  So the retail's two forms are a property of
 * the unit, and this cc1 reaches neither with a flag: the split form is the
 * "split 64-bit la" wall of docs/COMPILER-NOTES.md (no spelling measured reproduces
 * it) and the "base == dest" form is what -mno-split-addresses gives.
 *
 * The residue 3 (the second float of the pair at 0x160) is reachable: the retail
 * reads it as `0x4($v0)` with the address of the pair in $v0, so the source has to
 * keep the address in a register and read the second float through it
 * (`*(float *)((char *)p + 4)`), not as `p[1]` (which the compiler folds to
 * 0x164($s0)).
 */
