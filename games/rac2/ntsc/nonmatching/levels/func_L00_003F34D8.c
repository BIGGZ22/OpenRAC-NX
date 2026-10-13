/* func_L00_003F34D8: 8 instructions differ (register choice: retail keeps the count in $t1 and the quad temp in $v0; ours uses $v1/$a0). Quad copy is lq/sq via TImode. Missing: the allocator order. */
typedef unsigned int Quad_L00_003F34D8 __attribute__((mode(TI), aligned(16)));

void func_L00_003F34D8(Quad_L00_003F34D8 *a0, int *a1, Quad_L00_003F34D8 *a2, int *a3, int *a4)
{
    int n = *a4;

    if (n < 70) {
        a2[n] = *a0;
        a3[n] = *a1;
        *a4 = *a4 + 1;
    }
}
