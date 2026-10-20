/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x10 differs
   Mode byte at shared-table +8899: 0 converts the distance over the scale to a count, writes
   it through the out pointer and returns a0[2] + a1[2]*count - triangular(count)*scale; 1 and 2
   convert the func_L00_002D3B00 variant of the distance and return a0[2] + a1[2]*0.5f; outside
   0..2 returns 0.0f. The instruction text matches throughout; what differs is
   (a) this cc1 allocates the 2nd/3rd parameters to $s2/$s1 where retail has $s1/$s2, and the
   scale and the first conversion to $f20/$f21 where retail has $f21/$f20, and
   (b) the block order of the two symmetrical handlers: retail lands case 0 right after the
   `bltz`/`slti` guards and case 1-2 after it, this cc1 puts them the other way round.
   A switch form and a two-conversion form were also measured (see the alternatives); the
   switch form reaches 35 of 67 words, the if form 47 - neither reaches the allocation.
   What unblocks it: a lever that changes callee-saved allocation for two equally weighted
   parameters, or the block-order lever the sixth wave recorded as compiler-side. */

#include "include_asm.h"

extern char D_00189E20_19[];

extern int func_00283CF0(float);

extern float func_00283CE0(int);

/* Mode byte of the shared table: at 0 it converts the distance over the scale to a count,
   writes it to the out pointer and returns a displacement built from the count and its
   triangular number; at 1 or 2 it converts the mode-1/2 variant of the distance and returns
   half the displacement; otherwise 0.0f. */
float func_L00_002D3D98(float *a0, float *a1, int *out, float scale)
{
    char *p = D_00189E20_19;
    int mode = *(unsigned char *)(p + 8899);
    int n;

    if (mode == 0) {
        float a;
        float b;

        n = func_00283CF0(a1[2] / scale);
        a = func_00283CE0(n);
        b = func_00283CE0((n * n + n) >> 1);
        if (out)
            *out = n;
        return a0[2] + a1[2] * a - b * scale;
    }
    if (mode < 0)
        return 0.0f;
    if (mode >= 3)
        return 0.0f;
    n = func_00283CF0(func_L00_002D3B00((char *)a1) / scale);
    if (out)
        *out = n;
    return a0[2] + a1[2] * 0.5f;
}
