/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0xa4 differs
   ONE WORD from matching all 27 places: the closing 16-byte copy.
   Builds a 16-byte probe at a stack local from the shared block and its +0x80 matrix
   (func_00282D30(&local, base, -0.75f), func_00282CC0(&local, &local, base+0x80)), raises the
   third word by 0.5f, calls the level's float selector with that same 0.5f and STORES ITS
   RETURN back into the third word (that is what the second `swc1 $f0,0x8($sp)` in retail is -
   writing the two stores as one loses it), then tests
   func_00282C48(base[0x88] - local.f8) against 0.3f and, when it is not greater, calls
   func_L00_002D40A0(base+0x80, 1, base[0x98]) and copies the probe to base+0x80 with lq/sq.
   With the mode(TI) typedef the whole body matches: the lq is right and every other word is
   byte-identical. The single remaining word is
       retail 7E220000  sq $v0, 0x0($s1)      ours 7E020080  sq $v0, 0x80($s0)
   - this cc1 folds the constant destination offset into the sq instead of using the pointer
   retail materialises with `addiu $s1,$s0,0x80` (and which it does reuse for the call's $a0).
   Eight spellings measured, all folding identically: `char *dst`, `V4_5098 *dst`, no dst local
   at all (address written out at both sites), `*(V4_5098 *)(dst + 0)`, an alias `p = dst`,
   a TYPED store `= *(V4_5098 *)&local`, and both call/store permutations of dst vs base+0x80.
   What unblocks it: a lever that stops the destination address being rematerialised for a
   16-byte store; the sixth wave recorded the same fold on three other bodies (~30 places). */

#include "include_asm.h"

extern char D_00189E20_5098[];

extern void func_00282D30(char *dst, char *src, float k);

extern void func_00282CC0(void *out, void *in, void *m);

extern float func_002A89A8(char *a, int b, float f);

extern float func_00282C48(float);

extern void func_L00_002D40A0(char *a, int b, float f);

#ifndef RAC2_T_V4_5098
#define RAC2_T_V4_5098
typedef int V4_5098 __attribute__((mode(TI)));
#endif

void func_L00_002D5098(void) {
    char *base = D_00189E20_5098;
    char *dst = base + 0x80;
    V4_5098 local;
    float x;

    func_00282D30((char *)&local, base, -0.75f);
    func_00282CC0(&local, &local, dst);
    *(float *)((char *)&local + 8) = *(float *)((char *)&local + 8) + 0.5f;
    *(float *)((char *)&local + 8) = func_002A89A8((char *)&local, 0, 0.5f);
    x = func_00282C48(*(float *)(base + 0x88) - *(float *)((char *)&local + 8));
    if (0.3f < x)
        return;
    func_L00_002D40A0(dst, 1, *(float *)(base + 0x98));
    *(V4_5098 *)dst = local;
}
