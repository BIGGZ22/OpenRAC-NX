/* functional: medium. If D_0013D390[0x37] == 2 and D_0013D390[0x39] < 0: when D_0013D390[0x3B] is zero D_0015EFB0 = 14, else D_0015EFB0 = 17 and D_0015EFB4 |= 0x40. */
void func_L00_0024BA10(void) {
    if (D_0013D390[0x37] == 2 && D_0013D390[0x39] < 0) {
        if (D_0013D390[0x3B] == 0) {
            D_0015EFB0 = 14;
        } else {
            int f = D_0015EFB4;
            D_0015EFB0 = 17;
            D_0015EFB4 = f | 0x40;
        }
    }
}
