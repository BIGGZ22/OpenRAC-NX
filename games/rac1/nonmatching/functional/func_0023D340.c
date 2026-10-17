/* functional: medium. viBufAddDMA(ViBuf *). Same reads, writes and calls in the same order; a zero ring size (retail traps with break 7 on the divide) is treated as remainder 0 so the host does not crash. Field names are guesses from offsets. */
extern int func_00118CB0(int);
extern void func_00118C90(int);
extern void func_0023BF48(char *);
extern void func_0023CF80(unsigned int);
extern int func_0023CEC8(int *, unsigned int);
extern void func_0023CFF0(int, int, int, int);
extern char D_001E8E50[];

static int vb_mod(int a, int n) {
    return n == 0 ? 0 : a % n;
}

int func_0023D340(int *vb) {
    unsigned int chcr;
    unsigned int madr;
    int ret;
    int n;
    int r, newread;
    int sum, c, nblk, i, idx;
    int started = 0;

    func_00118CB0(vb[0x40 / 4]);
    if (vb[0x44 / 4] == 0) {
        func_0023BF48(D_001E8E50);
        return 0;
    }
    func_0023CF80(5);
    chcr = *(volatile unsigned int *)0x1000B400;
    madr = *(volatile unsigned int *)0x1000B410;
    ret = func_0023CEC8(vb, madr);
    n = vb[8 / 4];

    r = vb_mod(ret + n - vb[0xC / 4], n);
    newread = vb_mod(vb[0xC / 4] + r, n);
    vb[0x10 / 4] -= r;
    vb[0xC / 4] = newread;
    sum = newread + vb[0x10 / 4];
    idx = vb_mod(sum, n);

    c = vb[0x14 / 4];
    nblk = (c > -1 ? c : c + 0x7FF) >> 11;
    vb[0x14 / 4] = c - (nblk << 11);

    if (nblk > 0) {
        int j = vb_mod(sum + n - 1, n);
        started = 1;
        func_0023CFF0(vb[4 / 4] + (j << 4), vb[0] + (j << 11), 3, 0x80);
    }
    for (i = 0; i < nblk; ) {
        func_0023CFF0(vb[4 / 4] + (idx << 4), vb[0] + (idx << 11),
                      (i ^ (nblk - 1)) ? 3 : 0, 0x80);
        i++;
        idx = vb_mod(idx + 1, vb[8 / 4]);
    }

    vb[0x10 / 4] += nblk;
    if (vb[0x10 / 4] != 0) {
        if (started)
            chcr = (chcr & 0xFFFFFFF) | 0x30000100;
        func_0023CF80(chcr | 0x100);
    }
    func_00118C90(vb[0x40 / 4]);
    return 1;
}
