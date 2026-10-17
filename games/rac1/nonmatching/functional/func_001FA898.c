/* functional: high. corrected from nonmatching/game/func_001FA898.c: the C cast is undefined for out-of-range floats, while the EE's cvt.w.s saturates to 0x7FFFFFFF (large positive) or 0x80000000 (large negative); in-range values truncate toward zero as before. */
int func_001FA898(float f)
{
    if (f >= 2147483648.0f) return 0x7FFFFFFF;
    if (f < -2147483648.0f) return -2147483647 - 1;
    return (int)f;
}
