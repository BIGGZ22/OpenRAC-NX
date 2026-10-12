/* from src/core/0012C508.c: C that compiles but does not link at its places yet */
/* rac1-decomp src/core/00125630.c: func_00128968 */
/*
 * IPU command send: func_0012C3B0's wait (IPU_CTRL & 0x80004000 ==
 * 0x80000000, a r1_func_0012BCC8 kick every 5001 spins) inlined, then
 * cmd = arg1 | 0x40000000 to IPU_CMD, s+0x818 = r1_D_00132F70[cmd >> 28]
 * (func_0012C380 inlined), s+0x838 = the narrowed func_0012C458 result,
 * s+0x83C = 0x20.
 *
 * Written plainly it is exact under 2.9-ee, which rematerialises the
 * MMIO constants inside the loop and keeps only %hi(r1_D_00132F70) in $18,
 * like retail; 2.95.3 hoists them into saved registers (SIZE 252/240).
 * The stores go in source order 0x838 then 0x83C.
 */
void func_0012C788(void *arg0, int arg1) {
    int counter = 0;
    unsigned int cmd;

    while ((*(volatile int *)0x10002010 & 0x80004000) == 0x80000000) {
        if (counter++ >= 0x1389) {
            r1_func_0012BCC8(*(int *)((char *)arg0 + 0x858));
            counter = 0;
        }
    }
    cmd = arg1 | 0x40000000;
    *(volatile int *)0x10002000 = cmd;
    *(int *)((char *)arg0 + 0x818) = r1_D_00132F70[cmd >> 28];
    *(int *)((char *)arg0 + 0x838) = func_0012C458(arg0);
    *(int *)((char *)arg0 + 0x83C) = 0x20;
}
