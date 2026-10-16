/* NON_MATCHING func_L00_00238C30 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 2212 / retail 2164, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   QuickSelectDraw (hq9/s13): clamps two cnt-based ratios, computes a ring angle and calls func_00200198/func_002
 *   Still differs: i lives in $s5 (retail $s4), so the rec->unk74 == i compare and the loop registers are swapped;
 */
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_L00_0023BAB8(char *, int, int, int, int, int);
extern int func_00116248(char *, const char *, ...);
extern int func_001FA8A8(int, int, float);
extern float func_001FA748(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001F6EA8(int, int, long, void *, int);
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern int func_00116810(char *);
extern unsigned char D_0013E15A[];
extern char D_L00_001C43B0_c[] __asm__("D_L00_001C43B0");
extern short D_L00_0015F890;
extern short D_L00_0015F86C;
extern short D_L00_0015F8A4;
extern short D_L00_0015F8A0;
extern short D_L00_0015F800;
extern short D_L00_0015F804;
extern short D_L00_0015F870;
extern short D_L00_0015F874;
extern short D_L00_0015F878;
extern short D_L00_0015F87C;
extern short D_L00_0015F880;
extern short D_L00_0015F884;
extern short D_L00_0015F888;
extern short D_L00_0015F88C;
extern short D_L00_0015F894;
extern short D_L00_0015F898;
extern short D_L00_0015F89C;
extern int D_L00_0015F6B0 MACRO_ADDR;
extern short D_L00_0015F860;
extern char D_L00_0015F8A8[];
extern char D_0013D50F[];

/* Quick-select HUD slot 3 draw callback: lays out the eight quick-select entries around rec and returns rec->w. */
int func_L00_00238C30(HudElem *rec) {
    char buf[0x50];
    float f20, f21, f22, f23, ang, t1, t2;
    int a, b, sx, a9, hw, hh, n, i, o1, o2, o3, lim, xx, yy, k, e2, ii, s, F, F2, I, kk, s3, v, g, t3;
    char *p, *wr;
    unsigned char *G, *s0;
    unsigned char *c;
    unsigned char byte;
    int m;

    c = rec->cnt;
    f20 = func_001FA888(rec->cnt[0]);
    f22 = f20 / func_001FA888(*(int *)&D_L00_0015F860);
    if (1.0f < f22) {
        f22 = 1.0f;
    } else if (f22 < 0.0f) {
        f22 = 0.0f;
    }
    f20 = func_001FA888(c[1]);
    f23 = f20 / func_001FA888(*(int *)&D_L00_0015F860);
    if (1.0f < f23) {
        f23 = 1.0f;
    } else if (f23 < 0.0f) {
        f23 = 0.0f;
    }

    i = 0;
    m = D_L00_0015F6B0 % *(int *)&D_L00_0015F890;
    f20 = func_001FA888(m);
    f20 = f20 / func_001FA888(*(int *)&D_L00_0015F890);
    ang = f20 * 6.28318023681640625f - 3.141590118408203125f;
    f20 = func_001F9FA8(ang);
    f21 = f22 * 128.0f;
    sx = func_001FA898(f21);
    a9 = func_001FA898(f21 * (f20 * 0.125f + 0.875f));

    a = rec->unk50;
    b = rec->unk54;
    func_L00_00236400(rec, &a, &b);

    hw = *(int *)&D_L00_0015F7F8 / 2;
    hh = *(int *)&D_L00_0015F7FC / 2;
    func_00200468(func_00200198(0xE934, 0), a, b, hw, hh, sx);
    func_00200468(func_00200198(0xE934, 0), a + *(int *)&D_L00_0015F7F8, b + *(int *)&D_L00_0015F7FC, -hw, -hh, sx);
    func_00200468(func_00200198(0xE934, 0), a + *(int *)&D_L00_0015F7F8, b, -hw, hh, sx);
    func_00200468(func_00200198(0xE934, 0), a, b + *(int *)&D_L00_0015F7FC, hw, -hh, sx);

    n = D_L00_0015FB48;
    if (n > 0) {
        f21 = 3.14159265f;
        o1 = 0;
        o2 = 0;
        o3 = 0;
        for (; i < n; i++) {
            ang = ((float)i + (float)i) * f21 / (float)n - f21;
            f20 = func_001FA748(ang, 1.57079632679f);
            t1 = func_001F9F90(f20);
            xx = a + *(int *)&D_L00_0015F800 + (int)(*(float *)&D_L00_0015F86C * t1 * *(float *)&D_L00_0015F8A4);
            t2 = func_001F9FA8(f20);
            yy = b + *(int *)&D_L00_0015F804 + (int)(*(float *)&D_L00_0015F86C * t2);

            if (rec->unk74 == i) {
                if (*(int *)(((char **)D_L00_0015FB78)[*(int *)&D_L00_0015F80C] + o1)) {
                    func_00200468(func_00200198(0xE99C, 0), xx - 0x13, yy - 0x13, 0x26, 0x26, a9);
                }
            }
            if (*(int *)(((char **)D_L00_0015FB78)[*(int *)&D_L00_0015F80C] + o2)) {
                if (rec->unk74 == i) {
                    lim = sx;
                } else {
                    lim = func_001FA898((float)sx * *(float *)&D_L00_0015F8A0);
                }
                p = ((char **)D_L00_0015FB78)[*(int *)&D_L00_0015F80C] + o3;
                v = *(int *)(p + 0x18);
                byte = D_0013E15A[0x4C6 + v];
                t3 = func_00200198(*(int *)p, byte ? 4 : 0);
                func_L00_0023BAB8((char *)rec, t3, xx, yy, 1, lim);
            }
            n = D_L00_0015FB48;
            o1 += 0x1C;
            o2 += 0x1C;
            o3 += 0x1C;
        }
    }

    k = rec->unk74;
    if (k >= 0) {
        p = ((char **)D_L00_0015FB78)[*(int *)&D_L00_0015F80C] + k * 0x1C;
        if (*(int *)p) {
            f20 = f22 * f23;
            e2 = func_001FA8A8(*(int *)&D_L00_0015F878, *(int *)&D_L00_0015F87C, f20);
            ii = *(int *)((char *)D_0014171B_i + 0x885 + k * 4);
            wr = D_L00_001C43B0_c + ii * 24;
            if (*(unsigned short *)(wr + 8)) {
                s = *(int *)(D_0013D50F + 0x21 + ii * 4);
                func_00116248(buf, D_L00_0015F8A8, s, *(unsigned short *)(wr + 0xE));
                if (s) {
                    F = func_001FA8A8(*(int *)&D_L00_0015F888, *(int *)&D_L00_0015F88C, f20);
                } else {
                    F = func_001FA8A8(*(int *)&D_L00_0015F880, *(int *)&D_L00_0015F884, f20);
                }
                func_001F6EA8(a + *(int *)&D_L00_0015F800 + 1, b + *(int *)&D_L00_0015F804 + 13, e2, buf, -1);
                func_001F6EA8(a + *(int *)&D_L00_0015F800, b + *(int *)&D_L00_0015F804 + 12, F, buf, -1);
            }
            s3 = 0;
            F2 = func_001FA8A8(*(int *)&D_L00_0015F870, *(int *)&D_L00_0015F874, f20);
            p = ((char **)D_L00_0015FB78)[*(int *)&D_L00_0015F80C] + k * 0x1C;
            v = *(int *)(p + 0x18);
            byte = D_0013E15A[0x4C6 + v];
            if (byte) {
                g = *(short *)((char *)&D_L00_00179BC0[v] + 0x48);
            } else {
                g = *(short *)((char *)&D_L00_00179BC0[v] + 0x46);
            }
            G = func_001FE540_id(g);
            if (G) {
                if (func_00116810((char *)G)) {
                    I = func_00116810((char *)G);
                    s0 = G;
                    if (I > 0) {
                        kk = 1;
                        s0 = G + 1;
                        if (G[1] == '-') {
                            s0 = G + 2;
                            s3 = 2;
                        } else {
                            while (kk < I) {
                                s0++;
                                if (*s0 == '-') {
                                    s3 = kk + 2;
                                    s0++;
                                    break;
                                }
                                kk++;
                            }
                        }
                    }
                    if (s3 == 0) {
                        kk = 1;
                        s0 = G + 1;
                        if (s3 < I) {
                            if (G[1] == ' ') {
                                s0 = G + 2;
                                s3 = 1;
                            } else {
                                while (kk < I) {
                                    s0++;
                                    if (*s0 == ' ') {
                                        s3 = kk + 1;
                                        s0++;
                                        break;
                                    }
                                    kk++;
                                }
                            }
                        }
                    }
                    ii = *(int *)((char *)D_0014171B_i + 0x885 + k * 4);
                    wr = D_L00_001C43B0_c + ii * 24;
                    yy = b + *(int *)&D_L00_0015F804 + (*(unsigned short *)(wr + 8) ? *(int *)&D_L00_0015F894 : (s3 ? *(int *)&D_L00_0015F898 : *(int *)&D_L00_0015F89C));
                    if (s3 != 0) {
                        func_001F6EA8(a + *(int *)&D_L00_0015F800 + 1, yy + 1, e2, G, s3);
                        func_001F6EA8(a + *(int *)&D_L00_0015F800, yy, F2, G, s3);
                        yy += 0x10;
                        func_001F6EA8(a + *(int *)&D_L00_0015F800 + 1, yy + 1, e2, s0, -1);
                        func_001F6EA8(a + *(int *)&D_L00_0015F800, yy, F2, s0, -1);
                    } else {
                        func_001F6EA8(a + *(int *)&D_L00_0015F800 + 1, yy + 1, e2, G, -1);
                        func_001F6EA8(a + *(int *)&D_L00_0015F800, yy, F2, G, -1);
                    }
                }
            }
        }
    }
    return rec->w;
}
