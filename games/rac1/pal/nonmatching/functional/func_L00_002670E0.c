/* functional: medium. Written from the assembly; the a0 == 0 path is the tail of func_L00_00267120 (matched C in the packet: zeroes the 16-bit field at a1+0xA), taken here inline. */
extern char D_0014171B[];

void func_L00_002670E0(int a0, unsigned char *a1) {
    short v;
    if (a1 == 0) return;
    *(int *)a1 = a0;
    if (a0 == 0) {
        *(volatile unsigned short *)(a1 + 0xA) = 0;
        return;
    }
    v = *(short *)(a1 + 0xA);
    if (v == 1) {
        *(short *)(a1 + 0xA) = 4;
        if (*(short *)(a1 + 0x10) != 0) {
            *(short *)(D_0014171B + 0x100D5) = 1;
        }
    }
}
