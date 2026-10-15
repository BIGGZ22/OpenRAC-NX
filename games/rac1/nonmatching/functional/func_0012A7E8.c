/* functional: medium. libmpeg DMA of n*24 bytes (n = a1[3]*a1[4]) from the address in a1[0] to the IPU/SPR channel, split in two when over 0xFFFF; hardware polls kept as volatile reads. */
extern void *func_00118AC0(int, void *, int, void *);
extern void func_00119460(int);
extern void func_0012A558(void *, int, int);
extern void func_0012A718();

void func_0012A7E8(void *arg0, int *arg1) {
    char *p = (char *)arg0;
    int ev[8];
    int loc[4];
    int n, small, r;
    unsigned int mask = 0x0FFFFFFF;
    void *h;

    ev[0] = 2;
    n = arg1[3] * arg1[4];
    func_0012BC78(*(int *)(p + 0x858), ev);
    if ((*(volatile unsigned int *)0x10002010 & 0x4000) != 0) {
        *(volatile unsigned int *)0x10002010 = 0x40000000;
    }
    small = n < 0x400;
    while (*(volatile int *)0x10002010 < 0) {
    }
    func_00128560(arg0, 0);
    loc[0] = arg1[0];
    while (*(volatile int *)0x10002010 < 0) {
    }
    loc[1] = loc[0] & mask;
    loc[0] = n * 0x18;
    if ((unsigned int)loc[0] > 0xFFFF) {
        h = func_00118AC0(4, (void *)func_0012A718, 0, &loc[0]);
        *(volatile unsigned int *)0x1000E010 = 0x10;
        func_00119460(4);
        r = func_0011D960();
        *(volatile unsigned int *)0x1000B410 = loc[1];
        *(volatile unsigned int *)0x1000B420 = 0xFFFF;
        *(volatile unsigned int *)0x1000B400 = 0x101;
        if (r != 0) {
            func_0011D9A8();
        }
        loc[1] = (loc[1] + 0xFFFF0) & mask;
        loc[0] = loc[0] - 0xFFFF;
        if (small) {
            func_0012A2F0(arg0, *(int *)(p + 0xD8), n);
        } else {
            func_0012A558(arg0, *(int *)(p + 0xD8), n);
        }
        func_001193F8(4);
        func_00118AD0(4, (int)h);
    } else {
        r = func_0011D960();
        *(volatile unsigned int *)0x1000B410 = arg1[0] & mask;
        *(volatile unsigned int *)0x1000B420 = loc[0];
        *(volatile unsigned int *)0x1000B400 = 0x101;
        if (r != 0) {
            func_0011D9A8();
        }
        loc[0] = 0;
        if (small) {
            func_0012A2F0(arg0, *(int *)(p + 0xD8), n);
        } else {
            func_0012A558(arg0, *(int *)(p + 0xD8), n);
        }
    }
    ev[0] = 3;
    func_0012BC78(*(int *)(p + 0x858), ev);
}
