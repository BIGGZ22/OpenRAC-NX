/* functional: medium. Returns the stock D_0013D530[idx] (idx -1: the current item D_00140508) when the item's 24-byte cost record has a nonzero u16 at +0xE, else 1. */
extern int D_00140508_34718 __asm__("D_00140508");
extern char D_L00_001C43B0_34718[] __asm__("D_L00_001C43B0");
extern int D_0013D530_34718[] __asm__("D_0013D530");

int func_L00_00234718(int idx) {
    if (idx == -1) {
        idx = D_00140508_34718;
    }
    if (*(unsigned short *)(D_L00_001C43B0_34718 + idx * 24 + 0xE) == 0) {
        return 1;
    }
    return D_0013D530_34718[idx];
}
