/* Builds the scratchpad table of 16-byte records for p's current block
   (block index at +0x810, 0x140-byte blocks; the record count at +0x6BC of
   the block, the two word arrays at +0x598 and +0x5A8), then waits
   (func_0011D960) and points the DMA registers at the table: the last word
   of the +0x590 array, the table's real address, 0, and 0x105. If the wait
   returns nonzero, the result is func_0011D9A8's. The sync between the wait
   and the register writes has no C form here; the writes stay in order. */
extern char D_0015E940_b[] __asm__("D_0015E940");
extern int func_0011D9A8_r(void) __asm__("func_0011D9A8");

int func_0012A0F8(char *p) {
    int cnt = *(int *)(p + 0x810);
    int n = *(int *)(p + cnt * 0x140 + 0x6BC);
    unsigned int base = ((unsigned int)D_0015E940_b & 0x0FFFFFFFu) | 0x20000000u;
    unsigned int V;
    int r;
    int i;

    for (i = 0; i < n; i++) {
        int off = cnt * 0x140 + i * 4;
        unsigned int X = *(unsigned int *)(p + 0x5A8 + off);
        unsigned int Y = *(unsigned int *)(p + 0x598 + off);
        unsigned int at = base + 0x20u * (unsigned int)i;
        *(unsigned long *)at = ((unsigned long)(Y & 0x0FFFFFFFu) << 32) | 0x30000030UL;
        *(unsigned long *)(at + 0x10) = ((unsigned long)(X & 0x0FFFFFFFu) << 32)
                                        | (i == n - 1 ? 0UL : 0x30000000UL) | 0x30UL;
    }
    r = func_0011D960();
    V = *(unsigned int *)(p + 0x590 + cnt * 0x140);
    *(unsigned int *)0x1000D480u = V;
    *(unsigned int *)0x1000D430u = (unsigned int)D_0015E940_b;
    *(unsigned int *)0x1000D420u = 0;
    *(unsigned int *)0x1000D400u = 0x105;
    if (r == 0) {
        return r;
    }
    return func_0011D9A8_r();
}
