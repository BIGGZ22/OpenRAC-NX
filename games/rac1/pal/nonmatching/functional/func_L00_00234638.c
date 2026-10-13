/* functional: medium. Spend `amt` of item `idx` (-1: the current item D_00140508): returns 1 when the item has no cost record (u16 at +0xE of its 24-byte entry is 0) or the stock D_0013D530[idx] covers amt (then stock -= amt, D_0013DFA8[idx] += amt), else 0. Includes continuation piece func_L00_00234674. */
extern int D_00140508_34638 __asm__("D_00140508");
extern char D_L00_001C43B0_34638[] __asm__("D_L00_001C43B0");
extern int D_0013D530_34638[] __asm__("D_0013D530");
extern int D_0013DFA8_34638[] __asm__("D_0013DFA8");

int func_L00_00234638(int idx, int amt) {
    int v;
    if (idx == -1) {
        idx = D_00140508_34638;
    }
    if (*(unsigned short *)(D_L00_001C43B0_34638 + idx * 24 + 0xE) == 0) {
        return 1;
    }
    v = D_0013D530_34638[idx];
    if (v < amt) {
        return 0;
    }
    D_0013D530_34638[idx] = v - amt;
    D_0013DFA8_34638[idx] += amt;
    return 1;
}
