/* w13 ABANDON (almost) — func_L01_00397950, 212 bytes, 23 places.
 * last check.py line (5th compilation):
 *   func_L01_00397950: 0 of 23 places match
 *   (retail) "instruction at +0x8 differs"
 * Body identical to the retail instruction by instruction (two loops, order
 * of the 4 float stores 0x50/0x54/0x58/0x5C corrected), EXCEPT the allocation
 * of the two callee-saved: the retail puts the entity pointer ($68) in $s1
 * and the loop counter in $s2; this candidate does the opposite. $s0 (float *)
 * is correct in both. No source lever found: the declaration order (counter
 * first, last) changes nothing in the allocation.
 *
 * Order of the stores 0x50..0x5C (measured rule): the source [50, 54, 5C, 58]
 * gives the retail emission [5C, 58, 50, 54] (rotation by 2); the naive order
 * [50,54,58,5C] and the inverse order both gave a mismatch.
 */
extern int func_002A77E0(int);

void func_L01_00397950(char *a0)
{
    int i;
    char *s1 = *(char **)(a0 + 0x68);
    float *s0 = (float *)(s1 + 0x20);
    int *q;
    int v;

    for (i = 3; i >= 0; i--)
        *s0++ = (float)func_002A77E0(0xFF);

    *(float *)(s1 + 0x30) = -1.0f;
    *(float *)(s1 + 0x34) = -2.25f;
    *(float *)(s1 + 0x38) = 1.25f;
    *(float *)(s1 + 0x3C) = 2.5f;

    q = (int *)(s1 + 0x4C);
    v = 0xFF;
    for (i = 3; i >= 0; i--) {
        *q = v;
        q--;
        v -= 0x40;
    }

    *(float *)(s1 + 0x50) = 2.5f;
    *(float *)(s1 + 0x54) = 3.0f;
    *(float *)(s1 + 0x5C) = 2.5f;
    *(float *)(s1 + 0x58) = 3.0f;
}
