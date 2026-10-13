/* 72 bytes vs 76 (19 instructions): near miss. Retail keeps the 16-byte lq/sq temp in $v0 and
   folds the copy destination (0x18B6C0) into $v1, then rebases $v1 by -0x18A0 for the other
   stores; ours keeps the base in $v0 or $a2, so the temp register, the order and the lw differ. */
extern void func_L00_002C42E0(int, int);

void func_L00_002D40A0(V4_F6a70b950 *v, int mode, float t)
{
    char *base = (char *)&D_00189E20_11;
    char *dst = base + 0x18A0;
    int x;

    *(V4_F6a70b950 *)dst = *v;
    *(float *)(base + 0x18AC) = t;
    *(int *)(base + 0x18B0) = mode;
    x = *(int *)(base + 0x2294);
    if ((unsigned)(x - 98) >= 3)
        func_L00_002C42E0(98, 2);
}

