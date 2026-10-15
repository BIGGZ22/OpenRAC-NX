/* NON_MATCHING func_L00_001F3AF0 -- src/overlays/shared/draw_001F3A78.c
 * Best so far: SIZE ours 620 / retail 624, checked 2026-10-03.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   func_L00_001F3AF0: appends a line of font glyph quads (x, y, color, string) to the packet buffer D_L00_0016128
 *   p8.c (620 vs 624) has the same instruction sequence as retail: loop body, constants, stores all match in order
 *   Tried: one variable for packet pointer in prologue and loop (gives retail's `daddu $9,$2,$0`), `qw = 5` just b
 */
extern void func_00234C98(int, long);
extern int *D_L00_00161280 __attribute__((section(".sdata")));
extern long D_0015EFC8;
extern int D_L00_00169A80[];
extern int D_L00_00169C00[];
extern int D_0013E600[];

/* Appends a line of font glyph quads at (x, y) in COLOR to the packet buffer and patches the packet tags. */
void func_L00_001F3AF0(int x, int y, int color, unsigned char *str) {
    int n = 0;
    int qw;
    int *base;
    long *p;
    long *tag;
    int idx;
    int w;
    int c;
    long *q;

    func_00234C98(0x42, 0x8000000044L);
    func_00234C98(0x47, 0x5360B);
    base = D_L00_00161280;
    D_L00_00161280 = base + 4;
    p = (long *)D_L00_00161280;
    tag = p + 8;
    p[0] = 0x10ab400000000001L;
    p[1] = 0xE;
    p[2] = 1;
    p[3] = 0x14;
    p[4] = 0x2400000000000001L;
    p[5] = 0x61;
    p[6] = (long)color & 0xFFFFFFFFL;
    p[7] = D_0015EFC8;
    p[8] = 0x4400000000008003L;
    p[9] = 0x5353;
    D_L00_00161280 = (int *)((char *)D_L00_00161280 + 0x50);
    qw = 5;
    while ((c = *str) != 0) {
        p = (long *)D_L00_00161280;
        idx = (unsigned char)(c - 0x20);
        if (idx >= 0x60) {
            idx = 0x20;
        }
        w = D_L00_00169A80[idx];
        str++;
        if (w != -1) {
            p[0] = w;
            p[2] = w + 0xC000A0;
            p[1] = (x * 16 + D_0013E600[4] - 8)
                 | ((long)(y * 16 + D_0013E600[5] - 8) << 16)
                 | 0xFFFFF300000000L;
            p[3] = ((x + 10) * 16 + D_0013E600[4] - 8)
                 | ((long)((y + 12) * 16 + D_0013E600[5] - 8) << 16)
                 | 0xFFFFF300000000L;
            n++;
            qw += 2;
            D_L00_00161280 = (int *)((char *)D_L00_00161280 + 0x20);
        }
        x += D_L00_00169C00[idx];
    }
    base[3] = 0x50000000 | qw;
    base[0] = 0x10000000 | qw;
    base[1] = 0;
    base[2] = 0;
    *tag = ((long)n | 0x8000) | 0x4400000000000000L;
}
