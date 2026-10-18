/* functional: high. from PAL's func_0023A220 adapted to the US code: US checks different tables/strings (0x5233/0x5234/0x524E, D_0013D428, D_0013D4E3) and the else branch yields 0x524E where PAL gave 0, so the row is always drawn */
extern char D_001E63C0[];
extern char D_001DFFB0[];
extern int D_0013D428[];
extern unsigned char D_0013D4E3[];
extern int D_0015ED98;
extern void FUN_001fb8f0(int, int, int, int, int, int, int);
extern void *memset(void *, int, unsigned);
extern void *FUN_001fdd10(int);
extern void FUN_001f75f0(void *, long, void *, int);

void FUN_00238f08(int unused, int x, int y) {
    char sp[0x18];
    char *base = D_001E63C0;
    char *s = D_001DFFB0;
    char *e;
    int m, k, v;

    FUN_001fb8f0(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    memset(sp, 0, 0x18);
    *(short *)(sp + 0x8) = (short)(x >> 1);
    *(short *)(sp + 0xA) = (short)((y >> 1) - 7);
    *(short *)(sp + 0x10) = 0x10;
    *(short *)(sp + 0x12) = 1;
    *(short *)(sp + 0x2) = (short)y;
    *(short *)(sp + 0x6) = (short)x;

    if (*(int *)(base + 0x5C) == 0) {
        m = 0x5234;
    } else {
        e = base + *(int *)(base + 0x58) * 0x14;
        k = *(int *)(e + 0xD0);
        if (*(int *)(e + 0xD4) == 1 &&
            !(D_0013D428[k] < *(unsigned short *)(s + k * 0x18 + 0xE))) {
            m = 0x5233;
        } else if (*(int *)(e + 0xD4) == 1) {
            if (*(int *)(base + 0x40) == 0)
                v = *(unsigned short *)(s + k * 0x18 + 0x8);
            else
                v = *(unsigned short *)(s + k * 0x18 + 0xA);
            m = (D_0015ED98 < v) ? 0x5233 : 0x5234;
        } else {
            if (D_0013D4E3[0] == 0)
                v = *(int *)(s + k * 0x18);
            else
                v = *(int *)(s + k * 0x18 + 0x4);
            m = (D_0015ED98 < v) ? 0x5233 : 0x524E;
        }
    }

    if (m != 0) {
        *(short *)(sp + 0x12) = (short)(*(unsigned short *)(sp + 0x12) | 4);
        FUN_001f75f0(sp, 0x80F0F0F0L, FUN_001fdd10(0x5234), -1);
        *(short *)(sp + 0xA) = (short)((y - *(short *)(sp + 0xE)) >> 1);
        *(short *)(sp + 0x12) = (short)(*(unsigned short *)(sp + 0x12) ^ 4);
        FUN_001f75f0(sp, 0x80F0F0F0L, FUN_001fdd10(0x5234), -1);
    }
}
