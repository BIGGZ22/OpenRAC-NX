/* functional: medium. Menu state step: if the card word at 0x13D3AC is not -2 sets state 3; else folds in the following pieces func_L00_0024B980 / func_L00_0024B9B4 / func_L00_0024B9A8: flag 0x20 clears it and sets state 5 (or 0x17 if D_L00_0015F688 != 0); else flag 8 clears it and sets state 7. */
extern int D_0015EFB4 MACRO_ADDR;
extern int D_0015EFB0 MACRO_ADDR;
extern char D_0013D355[];
extern int D_L00_0015F688_B960 __asm__("D_L00_0015F688");
void func_L00_0024B960(void) {
    int flags;
    if (*(int *)(D_0013D355 + 0x57) != -2) {
        D_0015EFB0 = 3;
        return;
    }
    flags = D_0015EFB4;
    if (flags & 0x20) {
        D_0015EFB4 = flags ^ 0x20;
        if (D_L00_0015F688_B960 == 0) {
            D_0015EFB0 = 5;
            return;
        }
        D_0015EFB0 = 0x17;
        return;
    }
    if (flags & 8) {
        D_0015EFB4 = flags ^ 8;
        D_0015EFB0 = 7;
    }
}
