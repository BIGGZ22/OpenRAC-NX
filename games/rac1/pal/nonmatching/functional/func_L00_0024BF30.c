/* functional: medium. Menu state step on the card block at 0x13D390: when word[0xDC]<3 and word[0xE4]<0: if word[0xEC]==0 sets state D_0015EFB0 = 1 (via the copy of func_L00_0024BA60 right after it), else clears word[0xFC], sets state 0x15 and ors 0x440 into the flags D_0015EFB4. */
extern int D_0015EFB4 MACRO_ADDR;
extern int D_0015EFB0 MACRO_ADDR;
extern char D_0013D355[];
void func_L00_0024BF30(void) {
    int *b = (int *)(D_0013D355 + 0x3B);
    int flags;
    if (b[0xDC / 4] < 3) {
        if (b[0xE4 / 4] < 0) {
            if (b[0xEC / 4] == 0) {
                D_0015EFB0 = 1;
                return;
            }
            flags = D_0015EFB4;
            b[0xFC / 4] = 0;
            D_0015EFB0 = 0x15;
            D_0015EFB4 = flags | 0x440;
        }
    }
}
