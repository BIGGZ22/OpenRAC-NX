/* functional: medium. Part of libmpeg's DMA copy (the chunked sibling of func_0012A2F0): sets up a progress record (chunk counter, size/0x3FF+1 chunks, next source), installs the DMA callback func_0012A418 on channel 3 with the record, programs D3_MADR/QWC/CHCR, waits until the callback has counted all chunks, then finishes. */
extern int func_00118AC0_2A558(int, void *, int, void *) __asm__("func_00118AC0");
extern void func_00119460_2A558(int) __asm__("func_00119460");
extern int func_0011D960_2A558(void) __asm__("func_0011D960");
extern void func_0011D9A8_2A558(void) __asm__("func_0011D9A8");
extern void func_0012BC78_2A558(int, void *) __asm__("func_0012BC78");
extern void func_0012C468_2A558(void *, void *) __asm__("func_0012C468");
extern void func_001193F8_2A558(int) __asm__("func_001193F8");
extern void func_00118AD0_2A558(int, int) __asm__("func_00118AD0");
extern void func_0012A418_2A558(void) __asm__("func_0012A418");
extern char D_00153AB0_2A558[] __asm__("D_00153AB0");
void func_0012A558(char *ctx, unsigned int src, int size) {
    int event[8];
    volatile int rec[8];
    int handle, r, flag, total;

    rec[0] = 0;
    rec[1] = 0;
    rec[2] = size;
    rec[3] = (src + 0xFFC00) & 0x0FFFFFFF;
    rec[4] = size / 0x3FF + 1;
    while (*(volatile int *)0x10002010 < 0) {
    }
    handle = func_00118AC0_2A558(3, (void *)func_0012A418_2A558, 0, (void *)rec);
    *(volatile unsigned int *)0x1000E010 = 8;
    func_00119460_2A558(3);
    r = func_0011D960_2A558();
    *(volatile unsigned int *)0x1000B010 = src & 0x0FFFFFFF;
    *(volatile unsigned int *)0x1000B020 = 0xFFC0;
    *(volatile unsigned int *)0x1000B000 = 0x100;
    if (r != 0) {
        func_0011D9A8_2A558();
    }
    *(volatile unsigned int *)0x10002000 = 0x700003FF;
    event[0] = 4;
    func_0012BC78_2A558(*(int *)(ctx + 0x858), event);
    flag = rec[1];
    total = rec[4];
    while (rec[0] < total) {
    }
    if (flag != 0) {
        func_0012C468_2A558(ctx, D_00153AB0_2A558);
    }
    while (*(volatile int *)0x10002010 < 0) {
    }
    func_001193F8_2A558(3);
    func_00118AD0_2A558(3, handle);
}
