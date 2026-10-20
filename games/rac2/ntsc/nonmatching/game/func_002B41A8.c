/* func_002B41A8 (boot 0x2B41A8, 788 bytes, 28 places): NEAR MISS. This plain C compiles to 756 bytes (189
 * instructions) against retail's 788 (197); `tools/check.py` matches 0 of 28 places.
 *
 * What differs, and why it stays out of reach of plain C:
 *   - The four 16-byte copies, 8 instructions. Retail does each as `lq $v0,0(src)` / `sq $v0,0(dst)` with BOTH
 *     addresses first materialised in registers (`addiu $v1,$s0,0x10` before the first `sq`, `addiu $v1,$s1,0x10` before
 *     the `lq` of the else arm, `addiu $v1,$s0,0x20` in the delay slot of the `beqz $s3`, `addiu $a0,$s1,0xF0` before the
 *     last `lq`), always through $v0. That is the shape of the old qcopy() macro (inline asm "lq $2,0x0(%1)\n\tsq
 *     $2,0x0(%0)" with "r" operands; see the qcopy entries in nonmatching/README.md). With `mode(TI)` copies cc1 folds
 *     the constant into the immediate (`sq $v0,0x10($s0)`), which drops those 8 instructions and moves everything after
 *     them. Spellings measured: TI cast copy, the same through a volatile TI, a named pointer that is also passed to a
 *     call (cc1 still addresses off the base register), a TI pointer parameter, a 16-byte struct copy (ldl/ldr/sdl/sdr
 *     + lw/sw) and an aligned(16) struct copy (two ld/sd). None materialises the address.
 *   - Everything else was measured with the old inline-asm helper in place of the four copies (NOT plain C, never for
 *     src/): the same C then compiles to 788 bytes and 35 of 197 words differ, all register allocation and order, no
 *     instruction missing. The cause that was pinned down: global.c ranks pseudos by
 *     floor_log2(refs) * refs / live_length. Retail has obj in $s1 and p2 in $s2; here p2 (4 refs / 21, 0.381) outranks
 *     obj (7 refs / 38, 0.368) and takes $s1, so about half of the 35 words are obj and p2 renamed. The rest: the `sh` of the
 *     0x34 flag is scheduled before the two argument spills (`sw $t0,0x70($sp)`, `sw $t2,0x74($sp)`) here and after them
 *     in retail, and some of the seven `movn` selects get $s1 where retail has $s4 (and the reverse) or reload another
 *     stack slot. Where the 0x34 flag update is written moves the ratio across the threshold: obj/p2 right but the
 *     vector-copy block wrong, or the block right but obj/p2 swapped; no position gives both.
 *   - Brute force that went into it (about 100,000 compiles, all with that helper): the order of the seven select
 *     statements (all 5040, twice), the order of the eight `oN = dst + k` initialisations (all 40320), the order of the
 *     seven vector copies plus the flag update (all 40320), the 8 positions of the flag update, in-place against new
 *     variables for the selects (128 subsets), the declaration order of the select variables (5040, no effect) and a
 *     hill-climb over the interleaved order. The order below is the best one found. The float constant is the spelling
 *     that encodes 0x3998825C (0.00029088825f; 0.00029088821f encodes ...5B).
 *
 * Shape: ORs 0x100 into the halfword at obj+0x34; with p2 it calls func_00283098(dst, p2, obj+0xC0) and
 * func_00282CC0(dst, dst, obj+0x10) and copies the 16 bytes at p2 to dst+0x10, otherwise it copies obj+0x10 to dst;
 * it copies p3 or obj+0xF0 to dst+0x20; then it fills seven float[3] arrays of the record at dst from the optional
 * pointers p4..p10, each falling back to one of six default vectors copied from boot data D_001AA060..D_001AA0B0
 * (D_001AA090 serves two); the array at dst+0x84 is filled from three calls of the random-angle function
 * func_002A7940 and the one at dst+0x90 is scaled by 0.00029088825f (about pi/10800). The eleven parameters are a0..a3,
 * t0..t3 and three words on the stack (0x150/0x158/0x160($sp) in this frame).
 */

extern void func_00282CC0(void *, void *, void *);

typedef int TI16_2B41A8 __attribute__((mode(TI)));
typedef struct { float x, y, z; } Vec3_2B41A8;
extern Vec3_2B41A8 D_001AA060, D_001AA070, D_001AA080, D_001AA090, D_001AA0A0, D_001AA0B0;
extern float func_002A7940(void);
extern void func_00283098(void *, void *, void *);

void func_002B41A8(char *obj, char *dst, char *p2, char *p3, float *p4, float *p5, float *p6, float *p7,
                   float *p8, float *p9, float *p10)
{
    Vec3_2B41A8 v0, v1, v2, v3, v4, v5, v6;
    float *a, *b, *c, *d, *e, *f, *g;
    float *o0, *o1, *o2, *o3, *o4, *o5, *o6, *o7;
    int i;
    v0 = D_001AA060;
    v1 = D_001AA070;
    v2 = D_001AA080;
    v3 = D_001AA090;
    v4 = D_001AA0A0;
    v5 = D_001AA090;
    *(unsigned short *)(obj + 0x34) |= 0x100;
    v6 = D_001AA0B0;
    if (p2 != 0) {
        func_00283098(dst, p2, obj + 0xC0);
        func_00282CC0(dst, dst, obj + 0x10);
        *(TI16_2B41A8 *)(dst + 0x10) = *(TI16_2B41A8 *)p2;
    } else {
        *(TI16_2B41A8 *)dst = *(TI16_2B41A8 *)(obj + 0x10);
    }
    if (p3 != 0) {
        *(TI16_2B41A8 *)(dst + 0x20) = *(TI16_2B41A8 *)p3;
    } else {
        *(TI16_2B41A8 *)(dst + 0x20) = *(TI16_2B41A8 *)(obj + 0xF0);
    }
    c = p8 ? p8 : (float *)&v2;
    g = p5 ? p5 : (float *)&v6;
    d = p10 ? p10 : (float *)&v3;
    e = p9 ? p9 : (float *)&v4;
    b = p7 ? p7 : (float *)&v1;
    a = p6 ? p6 : (float *)&v0;
    f = p4 ? p4 : (float *)&v5;
    o7 = (float *)(dst + 0x90);
    o6 = (float *)(dst + 0x9C);
    o5 = (float *)(dst + 0x84);
    o4 = (float *)(dst + 0x6C);
    o3 = (float *)(dst + 0x54);
    o2 = (float *)(dst + 0x60);
    o1 = (float *)(dst + 0x48);
    o0 = (float *)(dst + 0x78);
    for (i = 0; i < 3; i++) {
        *o0++ = *d++;
        *o1++ = *a++;
        *o2++ = *e++;
        *o3++ = *b++;
        *o4++ = *c++;
        *o5++ = func_002A7940();
        *o6++ = *f++;
        *o7++ = *g++ * 0.00029088825f;
    }
}
