/* functional: medium. Builds the TLB from the table D_001303E8 { count0, count1, count2, wired, tab0, tab1, tab2 }: prints the layout with func_0011A690 (same six arguments as retail), clears Wired, writes every 16-byte entry of tab0, tab1 (indices continue), sets D_001303E8[3] and Wired to the number written, then tab2, and finally fills the remaining indices up to 0x2F with empty entries whose second word is 0xE0000000 + (idx << 13), idx being the count after all three groups, stepping by 0x2000; any group that would run past index 0x30 prints its message and calls func_0011DDA0(1). Wired (COP0 reg 6) has no host equivalent and is not written. Returns idx << 13 (retail leaves that in v0). The TLB-write syscall func_0011D490 argument order (index, w0, w1, w2, w3) and the messages D_00152A90/AC8/AE0/AF8 are inferred. */
extern int D_001303E8[];
extern char D_00152A90[];
extern char D_00152AC8[];
extern char D_00152AE0[];
extern char D_00152AF8[];
extern int func_0011A690(const char *fmt, ...);
extern void func_0011DDA0(int status);
extern void func_0011D490(int idx, unsigned a, unsigned b, unsigned c, unsigned d);

static int fill_11D4E0(int idx, int count, const unsigned *tab) {
    for (; idx < count; idx++) {
        func_0011D490(idx, tab[0], tab[1], tab[2], tab[3]);
        tab += 4;
    }
    return idx;
}

int func_0011D4E0(void) {
    int *g = D_001303E8;
    int a = g[0];
    int idx = 0;
    int n, i;
    unsigned addr;

    func_0011A690(D_00152A90, a - 1, a, (g[0] + g[1]) - 1, g[0] + g[1], (g[0] + g[1] + g[2]) - 1);

    n = g[0];
    if (n > 0x30) {
        func_0011A690(D_00152AC8);
        func_0011DDA0(1);
    }
    idx = fill_11D4E0(idx, n, (const unsigned *)g[4]);

    n = idx + g[1];
    if (n > 0x30) {
        func_0011A690(D_00152AE0);
        func_0011DDA0(1);
    }
    idx = fill_11D4E0(idx, n, (const unsigned *)g[5]);

    g[3] = idx;
    if (g[2] > 0) {
        n = idx + g[2];
        if (n > 0x30) {
            func_0011A690(D_00152AF8);
            func_0011DDA0(1);
        }
        idx = fill_11D4E0(idx, n, (const unsigned *)g[6]);
    }

    addr = ((unsigned)idx << 13) + 0xE0000000u;
    for (i = idx; i < 0x30; i++) {
        func_0011D490(i, 0, addr, 0, 0);
        addr += 0x2000;
    }
    return idx << 13;
}
