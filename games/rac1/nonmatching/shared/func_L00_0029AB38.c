/* NON_MATCHING func_L00_0029AB38 -- src/overlays/shared/tieproc_00299108.c
 * Best so far: BYTES 88/480 (81.7% of the bytes match), checked 2026-10-06.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   func_L00_0029AB38 (MovieExitToGameplay): sets D_L00_0015F6BC, resets the loader, then two frame-drawing loops 
 *   Best is p6.c: frame size and all instruction shapes agree (movn for the counts, MACRO_ADDR gp-in-delay-slot ac
 *   Retail has x=$s0, y=$s1, loop counter=$s2 (and the store of D_L00_0015F6BC sits in the first jal's delay slot,
 *   Tried counter as for(i<m), for(i=m;i;i--), while(m--), giv forms for x; declaration order does not matter. Unb
 *   t11 round (p10-p13): p10/p13 (80/480 bytes, size now right: sp[0x70], y loaded before x, p[2]=0 before p[3]=0)
 */
extern int D_L00_00161118 MACRO_ADDR;
extern int D_L00_0015F6A8 MACRO_ADDR;
extern void func_00122598(int);
extern void func_00120858(int, int);
extern void func_00123168(void *);
extern void func_0012F308(void);
extern void func_00122818(void *, int, int, int, int, int, int, int);
extern void func_00122AD8(void *, int);
extern void func_00216A90(int, int, int);
extern int D_0015EF88 MACRO_ADDR;
extern short D_0015EF78;
extern short D_L00_0015F6BC_g __asm__("D_L00_0015F6BC");

/* Leaves a movie: runs the two fade-out loops that draw frames, clears the movie flags and restores the player's flag. */
void func_L00_0029AB38(void) {
    int x, y;
    int n, m;
    char sp[0x60];
    int i;
    int *p;
    char *q;

    *(int *)&D_L00_0015F6BC_g = 1;
    func_00122598(0);
    func_00120858(0, 0);
    func_00123168(func_0012F308);
    x = D_0015EF88;
    y = D_L00_00161118;
    m = 0xD;
    if (D_0015EE80 != 0) {
        m = 0xE;
    }
    n = 0x11;
    if (D_0015EE80 != 0) {
        n = 0xE;
    }
    for (i = 0; i < m; i++) {
        func_00122818(sp, (x << 8) >> 16, 2, 1, 0, 0, 0x80, 0x80);
        x += 0x10000;
        func_00118D80(0);
        func_00122AD8(sp, y);
        y += 0xC000;
        func_00120858(0, 0);
    }
    x = *(int *)&D_0015EF78;
    for (i = 0; i < n; i++) {
        func_00122818(sp, (x << 8) >> 16, 1, 0, 0, 0, 0x40, 0x40);
        x += 0x4000;
        func_00118D80(0);
        func_00122AD8(sp, y);
        y += 0x4000;
        func_00120858(0, 0);
    }
    if (D_0015EFD8_m == 2) {
        *(int *)&D_0015EFD8 = 0;
    }
    func_001F4E08(4);
    q = D_0014171B + 0x100B5;
    func_00216A90(*(short *)(q + 0x38), 1, 0x400);
    D_L00_0015F6A8 = 0;
    p = D_L00_00179200;
    if (p[2] != 0) {
        int a = p[2];
        int b = p[3];
        p[3] = 0;
        p[2] = 0;
        func_00217AE8(a, b, 1);
    }
    q = D_0013E633 + 0x1D;
    q[0x6B] |= 0x10;
}
