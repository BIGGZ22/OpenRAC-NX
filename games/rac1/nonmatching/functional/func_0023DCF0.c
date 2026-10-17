/* functional: medium. Searches a ring of 24-byte entries (obj+0x50, count obj+0x54, live count obj+0x58, head obj+0x5C) for one whose due time has passed relative to the DMA/IPU address state, moves its 16 bytes to out and invalidates it; same reads/writes/calls in order as retail, the retail divide-by-zero break traps are replaced by an early skip; field meaning is inferred from the assembly only. */
extern int func_00118CB0(int);
extern void func_00118C90(int);

int func_0023DCF0(int *obj, long *out) {
    unsigned int a = *(volatile unsigned int *)0x1000B410;
    unsigned int ipu = *(volatile unsigned int *)0x10002020;
    unsigned int s17 = (unsigned int)obj[0x38 / 4] & 0x7F;
    int stride = obj[8 / 4];
    int h = obj[0x40 / 4];
    unsigned int s19 = (unsigned int)stride << 11;
    unsigned int s16;
    int found = 0;

    a -= (((ipu >> 16) & 3) + ((ipu >> 8) & 0xF)) << 4;
    func_00118CB0(h);
    out[1] = -1;
    out[0] = -1;
    s16 = a + (s17 >> 3) + s19;
    if (s19 != 0) {
        int cnt0 = obj[0x58 / 4];
        int base;
        unsigned int mod;
        int s10, i;
        s16 -= (unsigned int)obj[0];
        mod = s16 % s19;
        base = obj[0x5C / 4];
        if (cnt0 > 0) {
            int t = (int)(mod + s19);
            s10 = base - cnt0;
            i = 0;
            do {
                int ring = obj[0x54 / 4];
                char *e;
                int idx, d;
                if (ring == 0)
                    break;
                idx = (s10 + ring + i) % ring;
                e = (char *)obj[0x50 / 4] + idx * 24;
                d = *(int *)(e + 0x10);
                i++;
                if (((t - d) % (int)s19) < *(int *)(e + 0x14)) {
                    int cnt;
                    found = 1;
                    out[0] = *(long *)e;
                    e = (char *)obj[0x50 / 4] + idx * 24;
                    out[1] = *(long *)(e + 8);
                    e = (char *)obj[0x50 / 4] + idx * 24;
                    *(long *)e = -1;
                    e = (char *)obj[0x50 / 4] + idx * 24;
                    *(long *)(e + 8) = -1;
                    cnt = obj[0x58 / 4];
                    obj[0x58 / 4] = cnt >= 2 ? cnt - 1 : 0;
                }
            } while (i < cnt0 && !found);
        }
    }
    func_00118C90(obj[0x40 / 4]);
    return 1;
}
