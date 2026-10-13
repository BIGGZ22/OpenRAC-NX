/* func_L00_003B0A28, 136 bytes, 27 placements.  ABANDON.
   Last line of tools/check.py: "00_aranos_tutorial: 116 bytes, expected 136"
   Retail walks the slot table with the cursor in $a1, the next cursor in $a2
   and loop-invariant copies in $a3 (the first cursor) and $t1 (the table),
   emitting the cursor store twice - once in the bnel delay slot, once after the
   wrap-around exit.  The source below reproduces the single duplicated store
   but not the register copies. */

extern char D_L00_0023BB78[];
extern unsigned char D_L00_001AF810 __attribute__((sda));
extern unsigned char D_L00_001AF811 __attribute__((sda));

void func_L00_003B0A28(void)
{
    unsigned char first = D_L00_001AF810;
    char *base = D_L00_0023BB78;
    char *table = base + 0xC;
    unsigned char c;
    int i;

    if (D_L00_001AF811 == first)
        return;
    c = D_L00_001AF811 + 1;
    D_L00_001AF811 = c;
    if (*(int *)(table + c * 0x1C) != 0)
        return;
    if (c == first)
        return;
    for (i = c; ; ) {
        i = i + 1;
        if (*(int *)(table + (i & 0xFF) * 0x1C) != 0)
            break;
        if ((i & 0xFF) == first)
            break;
    }
    D_L00_001AF811 = i;
}
