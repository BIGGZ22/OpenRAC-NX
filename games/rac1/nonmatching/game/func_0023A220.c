/* Draws one vendor list row at (x = a1, y = a2): a 0x18-byte record on the
 * stack holds the position, the row choice comes from the vendor state at
 * D_001E66C0 and the tables D_001E02B0 (0x18-byte rows), D_0013D530 and
 * D_0013D5EB, and the row is drawn twice (func_001F75D0). The first argument
 * is not used. */
extern unsigned char D_0013D5EB[];
extern int D_0013D530[];
extern int D_0015EE98 MACRO_ADDR;
extern void func_001153FC(void *, int, int);
extern void func_001F75D0(void *, long, void *, int);

void func_0023A220(int unused, int x, int y) {
    char sp[0x18];
    char *base = D_001E66C0;
    char *s = (char *)D_001E02B0_p;
    char *e1;
    void *r1;
    void *r2;
    int m;
    int idx;
    int k;
    int v;

    func_001FBAB8(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    func_001153FC(sp, 0, 0x18);
    *(short *)(sp + 0x8) = (short)(x >> 1);
    *(short *)(sp + 0xA) = (short)((y >> 1) - 7);
    *(short *)(sp + 0x10) = 0x10;
    *(short *)(sp + 0x12) = 1;
    *(short *)(sp + 0x2) = (short)y;
    *(short *)(sp + 0x6) = (short)x;

    if (*(int *)(base + 0x5C) == 0) {
        m = 0x5239;
    } else {
        idx = *(int *)(base + 0x58);
        e1 = base + idx * 0x14;
        k = *(int *)(e1 + 0xD0);
        if (*(int *)(e1 + 0xD4) == 1) {
            if (D_0013D530[k] < *(unsigned short *)(s + k * 0x18 + 0xE)) {
                if (*(int *)(base + 0x40) == 0) {
                    v = *(unsigned short *)(s + k * 0x18 + 0x8);
                } else {
                    v = *(unsigned short *)(s + k * 0x18 + 0xA);
                }
                m = (D_0015EE98 < v) ? 0x5238 : 0x5239;
            } else {
                m = 0x5238;
            }
        } else {
            if (D_0013D5EB[0] == 0) {
                v = *(int *)(s + k * 0x18);
            } else {
                v = *(int *)(s + k * 0x18 + 0x4);
            }
            m = (D_0015EE98 < v) ? 0x5238 : 0;
        }
    }

    if (m != 0) {
        *(short *)(sp + 0x12) = (short)(*(unsigned short *)(sp + 0x12) | 4);
        r1 = func_001FE540_id(m);
        func_001F75D0(sp, 0x80F0F0F0L, r1, -1);
        *(short *)(sp + 0x12) = (short)(*(unsigned short *)(sp + 0x12) ^ 4);
        *(short *)(sp + 0xA) = (short)((y - *(short *)(sp + 0xE)) >> 1);
        r2 = func_001FE540_id(m);
        func_001F75D0(sp, 0x80F0F0F0L, r2, -1);
    }
}
