extern char D_L00_001ABB60[];

/* Clears n bits of the bitmap at D_L00_001ABB60, from bit idx upward, one bit
   at a time (at least one). Retail checks each bit is set first and traps
   (teq) when it is already clear, storing nothing for that bit; this returns
   at that point instead. */
void func_L00_00251A78(unsigned int idx, int n) {
    do {
        unsigned char *p = (unsigned char *)D_L00_001ABB60 + (idx >> 3);
        unsigned int bit = 1u << (idx & 7);
        unsigned int old = *p;
        unsigned int nw = old & ~bit;
        if (nw == old) {
            return;
        }
        idx++;
        n--;
        *p = (unsigned char)nw;
    } while (n > 0);
}
