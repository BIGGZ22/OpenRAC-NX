/* NON_MATCHING func_L00_00248EF8 -- src/overlays/shared/map_002465F8.c
 * Best so far: SIZE ours 1796 / retail 1808, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped at budget (12 runs). Best p5.c is SIZE 1796 vs 1808 (12 bytes short); p0 was 1748, p4 1780, p6 1788. B
 *   Differences left: retail keeps gy (sp+0x48) and the box bounds in stack slots and reloads them for each callba
 */
extern int D_L00_00184318 MACRO_ADDR;
extern short D_L00_0015FEB8;
extern int D_0015EE84 MACRO_ADDR;
extern char D_0013E633[];
extern char D_L00_00182CA0[];
extern char D_L00_00182CA4[];
extern char D_L00_00182CA8[];
extern char D_L00_001845A8[];
extern char D_L00_00184090[];
extern char D_L00_00184094[];
extern char D_L00_00184098[];
extern char D_L00_0018409C[];
extern char D_L00_001840A0[];
extern char D_L00_001840A4[];
extern char D_L00_001840A8[];
extern char D_L00_001840AC[];
extern char D_L00_001842F0[];
extern int func_L00_0024A798(int);
extern void func_L00_0024B200(float, float, float *, float *, int);
typedef int (*Cb5)(float, float, float, int, int);

// Clears the visibility bits of the 16x16 cells around the point v projects to, after the per-cell tests and callbacks.
void func_L00_00248EF8(float *v) {
    int flags[16];
    int *fl;
    float ox, oy;
    int gx, gy, lvl, f4c, mode, m10, m0f, yraw0, yraw1, xraw0, xraw1, i, i16, s, off, mask, x, x0, x1, y, y0, y1, m1, ax, bx, sel, row4, sh, b, bit, cell;
    char *g, *fb, *p, *bitp;
    Cb5 fp;

    if (D_L00_00184318 == 0) return;
    if (*(int *)&D_L00_0015FEB8 != 0) {
        func_L00_0024B200(v[0], v[1], &ox, &oy, D_0015EE84 + 0x64);
    } else {
        func_L00_0024B200(v[0], v[1], &ox, &oy, D_0015EE84);
    }
    gx = (int)(ox * 512.0f) + 1;
    gy = (int)(oy * 512.0f) + 1;
    if (gx < 0 || gy < 0) return;
    if (gx >= 0x200 || gy >= 0x200) return;

    g = D_0013E633 + 0xE1D;
    lvl = D_0015EE84;
    if (lvl < 0) lvl = 0;
    if (lvl >= 0x13) lvl = 0;
    f4c = 0;
    mode = *(int *)(g + 0x208C);
    if ((unsigned)(mode - 0x11) < 2 || *(unsigned char *)(g + 0x12E4) == 1) f4c = 1;

    m10 = (mode == 0x10);
    m0f = (mode == 0xF);
    yraw0 = gy - 16;
    xraw0 = gx - 16;
    yraw1 = gy + 16;
    xraw1 = gx + 16;
    for (i = 0, i16 = 0, fl = flags; i < 16; i++, i16 += 16, fl++) {
        *fl = 1;
        if (i <= 0) continue;
        s = lvl << 8;
        off = s + (i << 4);
        if (*(float *)(D_L00_00182CA0 + off) != *(float *)(D_L00_00182CA4 + off)) {
            p = *(char **)(g + 0x2080);
            if (*(float *)(p + 0x18) < *(float *)(D_L00_00182CA0 + off) || *(float *)(D_L00_00182CA4 + off) < *(float *)(p + 0x18)) {
                *fl = 0;
                continue;
            }
        }
        mask = *(int *)(D_L00_00182CA8 + i16 + s);
        if (mask == 0) continue;
        if ((mask & 1) && f4c == 0) { *fl = 0; continue; }
        if ((mask & 2) && f4c != 0) { *fl = 0; continue; }
        if ((mask & 4) && !m10) { *fl = 0; continue; }
        if ((mask & 8) && m10) { *fl = 0; continue; }
        if ((mask & 0x10) && !m0f) { *fl = 0; continue; }
        if ((mask & 0x20) && m0f) { *fl = 0; continue; }
        if ((mask & 0x40) && *(short *)(g + 0x308) == 0) { *fl = 0; continue; }
        if ((mask & 0x80) && *(int *)(D_L00_001845A8 + (*(int *)(D_L00_00182CA0 + off + 0xC) << 2)) == 0) { *fl = 0; continue; }
        if (mask & 0x100) {
            fp = *(Cb5 *)(D_L00_00184090 + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x200) {
            fp = *(Cb5 *)(D_L00_00184094 + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x400) {
            fp = *(Cb5 *)(D_L00_00184098 + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x800) {
            fp = *(Cb5 *)(D_L00_0018409C + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x1000) {
            fp = *(Cb5 *)(D_L00_001840A0 + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x2000) {
            fp = *(Cb5 *)(D_L00_001840A4 + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x4000) {
            fp = *(Cb5 *)(D_L00_001840A8 + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
        if (mask & 0x8000) {
            fp = *(Cb5 *)(D_L00_001840AC + (D_0015EE84 << 5));
            if (fp != 0 && fp(v[0], v[1], v[2], gx, gy) == 0) { *fl = 0; continue; }
        }
    }

    y0 = yraw0;
    if (y0 < 0) y0 = 0;
    y1 = yraw1;
    if (y1 >= 0x200) y1 = 0x1FF;
    x0 = xraw0;
    if (x0 < 0) x0 = 0;
    x1 = xraw1;
    if (x1 >= 0x200) x1 = 0x1FF;
    if (y0 >= y1) return;

    for (y = y0; y < y1; y++) {
        row4 = (y - gy + 16) * 4;
        yraw0 = gy - 16;
    xraw0 = gx - 16;
    yraw1 = gy + 16;
    xraw1 = gx + 16;
    m1 = -1;
        ax = x0 + 0x17 - gx;
        bx = x0 + 0x10 - gx;
        for (x = x0; x < x1; x++, ax++, bx++) {
            sel = ax;
            if (m1 < bx) sel = bx;
            fb = *(char **)(D_L00_001842F0 + 0xC);
            bitp = fb + (y << 6) + (x / 8);
            if (*bitp == 0) continue;
            bit = 1 << (x & 7);
            if ((*bitp & bit) == 0) continue;
            sh = bx - ((sel >> 3) << 3);
            b = *(unsigned char *)(D_L00_001842F0 + 0x1A4 + (sel >> 3) + row4);
            if (((b >> sh) & 1) != 0) continue;
            cell = (x >> 5) + ((y >> 5) << 4);
            func_L00_0024A798(cell);
            p = *(char **)(D_L00_001842F0 + 0x38);
            b = *(unsigned char *)(p + ((x & 31) >> 1) + ((y & 31) << 4));
            if (x & 1) b = b >> 4;
            else b = b & 0xF;
            if (flags[b] != 0) *bitp = *bitp & ~bit;
        }
    }
}
