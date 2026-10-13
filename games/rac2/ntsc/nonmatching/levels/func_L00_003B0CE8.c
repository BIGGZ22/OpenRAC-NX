/* func_L00_003B0CE8, 184 bytes, 27 placements.  ABANDON.
   Last line of tools/check.py: "00_aranos_tutorial: 168 bytes, expected 184"
   Retail clears slot[idx], then walks the table from the cursor to the first
   free slot, rewriting every byte equal to idx inside each busy slot's 8-byte
   id field, and finally calls func_L00_003B0A28 when the cursor wrapped back to
   idx.  This source reproduces the shape but not the two distinct 0x1C
   materialisations ($t1/$t2) nor the $t4 = base+0x14 base register. */

extern char D_L00_0023BB78[];
extern unsigned char D_L00_001AF810 __attribute__((sda));
extern unsigned char D_L00_001AF811 __attribute__((sda));

extern void func_L00_003B0A28();

void func_L00_003B0CE8(int idx)
{
    unsigned char c = idx;
    unsigned char first = D_L00_001AF810;
    unsigned char count = D_L00_001AF811;
    char *base = D_L00_0023BB78;
    char *table = base + 0xC;
    char *bytes = base + 0x14;

    *(int *)(table + c * 0x1C) = 0;
    if (count == first)
        return;
    for (;;) {
        if (*(int *)(table + count * 0x1C) != 0) {
            unsigned char *p = (unsigned char *)(bytes + count * 0x1C);
            int i;

            for (i = 7; i >= 0; i--) {
                if (*p == c)
                    *p = count;
                p++;
            }
        }
        count = count + 1;
        if (count != first)
            continue;
        break;
    }
    if (c == D_L00_001AF811)
        func_L00_003B0A28();
}
