/* functional: high. Clears bits 0x4 and 0x2 of D_0015EFB4 (each stored back), then: if 0x80 is set, D_0015EFB0 = 0x15 and D_0015EFB4 = (flags ^ 0x80) | 0x40; else if 0x100 is set, D_0015EFB0 = 0x14 and D_0015EFB4 = (flags ^ 0x100) | 0x40; else if the word at D_0013D355+0x3B+0x1C is 0, D_0015EFB0 = 1 when the word at +0xFC is nonzero; otherwise D_0015EFB0 = 3. Stores happen in the assembly's order. */
extern int D_0015EFB4 MACRO_ADDR;
extern int D_0015EFB0 MACRO_ADDR;
extern char D_0013D355[];
void func_L00_0024BD18(void) {
    int s = D_0015EFB4;
    char *q;
    if (s & 4) {
        s = s & -5;
        D_0015EFB4 = s;
        s = D_0015EFB4;
    }
    if (s & 2) {
        s = s & -3;
        D_0015EFB4 = s;
        s = D_0015EFB4;
    }
    if (s & 0x80) {
        D_0015EFB0 = 0x15;
        D_0015EFB4 = (s ^ 0x80) | 0x40;
        return;
    }
    if (s & 0x100) {
        D_0015EFB0 = 0x14;
        D_0015EFB4 = (s ^ 0x100) | 0x40;
        return;
    }
    q = D_0013D355 + 0x3B;
    if (*(int *)(q + 0x1C) == 0) {
        if (*(int *)(q + 0xFC) != 0) {
            D_0015EFB0 = 1;
        }
    } else {
        D_0015EFB0 = 3;
    }
}
