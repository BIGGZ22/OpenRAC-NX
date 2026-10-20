/* func_L20_00443A10 (332 bytes, level 20): near miss, 28 of 83 words differ.
 *
 * What matches: the size is exact (332 bytes), the whole block structure, every call, the
 * three float constants (0x3FC90FDB = 1.5707964f, the $gp constant D_L20_001B2098 at
 * gp+0x30A8 = 0x1B2098), the two func_00283248 argument pairs (the first reads the boot
 * table D_189E20 at +0x80/+0x84, the second the level table D_L20_001B308C at +0x30/+0x34),
 * the blend (func_00283BE0 / func_00283B98 against 1.5707964f), the three stores into
 * arg1 (0x0, 0x4, 0x8) and the last call func_00282CC0.
 *
 * The unit has to be the -G8 sibling (the constant is read through $gp) and it has to be
 * built with -mno-split-addresses: that is what gives the retail's absolute int load its
 * base == dest form (`lui $v1,%hi(D_1B308C); lw $v1,%lo($v1)`).
 *
 * Four residues, all measured:
 *  1. The address of the boot table: retail materialises it as two instructions the
 *     scheduler separates - `lui $v0,%hi(D_189E20)`, `sd $s0,0($sp)`,
 *     `addiu $v0,$v0,%lo(D_189E20)` - while -mno-split-addresses makes cc1 emit the single
 *     `la` pseudo (lui+addiu adjacent). Without the flag cc1 emits the split pair too, but
 *     then the absolute int loads lose their base == dest form and the body grows to 348
 *     bytes with a bigger frame. Both spellings of the base (a float* variable, a char*
 *     variable with +0x80/+0x84, &D[0], D[0]+0) give the same `la`.
 *  2. The register of the table index: retail loads it into $v0 and the table base into
 *     $v1, this cc1 the other way round ($v1 for the index, $v0 for the base).
 *  3. The blend result: retail keeps it in $f21 (the second call's result register) and this
 *     cc1 in $f20 (the first call's). Four words: the two `mov.s $f2x,$f0` and the two
 *     `mov.s $f12,$f2x`.
 *  4. The tail: retail sets up the third argument of the last call (the base load, the
 *     `daddu $a1,$a0,$zero`, the `mul.s` and the two stores) in a different order; the
 *     `sw $zero,0x8($s1)` comes after the `mul.s` in retail and before it here.
 *
 * Tried and not enough: the declaration order of the locals, the three stores permuted
 * (all six orders of the three stores), the blend into a named temporary, the two calls
 * inlined into the blend, the table pointer as a local and written out twice, the base
 * through a float*, a char* and &D[0]. The $gp constant is D_L20_001B2098 (gp + 0x30A8).
 */
#include "common.h"

typedef float f32;

extern float D_189E20[];

extern char *D_L20_001B308C __attribute__((nosda));

extern float D_L20_001B2098 __attribute__((sda));

extern float func_00283248(float, float);

extern float func_00283BE0(float, float);

extern float func_00283B98(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

extern void func_00282CC0(char *, char *, char *);

void func_L20_00443A10(char *arg0, char *arg1)
{
    char *s0 = arg0;
    char *s1 = arg1;
    char *state = *(char **)(s0 + 0x68);
    float *v = (float *)D_189E20;
    float a, b, c;

    a = func_00283248(v[0x20] - *(float *)(s0 + 0x10),
                      v[0x21] - *(float *)(s0 + 0x14));
    b = func_00283248(
        *(float *)((char *)D_L20_001B308C + (*(int *)(state + 0x230) << 7) + 0x30) -
            *(float *)(s0 + 0x10),
        *(float *)((char *)D_L20_001B308C + (*(int *)(state + 0x230) << 7) + 0x34) -
            *(float *)(s0 + 0x14));
    c = func_00283BE0(a, b);
    if (0.0f < c) {
        c = func_00283BE0(b, 1.5707964f);
    } else {
        c = func_00283B98(b, 1.5707964f);
    }
    *(float *)s1 = func_00283180(c) * D_L20_001B2098;
    *(float *)(s1 + 4) = func_00283198(c) * D_L20_001B2098;
    *(int *)(s1 + 8) = 0;
    func_00282CC0(s1, s1, (char *)((char *)D_L20_001B308C +
                                   (*(int *)(state + 0x230) << 7) + 0x30));
}
