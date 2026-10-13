/* functional: medium. Covers the func_L00_0024BA8C tail too, since the beqz at 0x24BA78 branches there. If the word at D_0013D355+0x57 is nonzero, store 3 to D_00158FB0 (gp-0x7D50) and return. If it is zero, the word at D_00158FB4 (gp-0x7D4C) is tested: when it has bits 1 or 2 set, D_0015EFB0 = 10. The gp load in the delay slot runs on both paths. */
extern char D_0013D355[];
extern int D_00158FB4;
extern int D_00158FB0;
extern int D_0015EFB0;

void func_L00_0024BA70(void) {
    int v = *(int *)(D_0013D355 + 0x57);
    if (v != 0) {
        D_00158FB0 = 3;
        return;
    }
    v = D_00158FB4;
    if (v & 6) {
        D_0015EFB0 = 10;
    }
}
